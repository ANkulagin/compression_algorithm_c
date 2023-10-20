#include <stdio.h>
// Специальные символы для обозначения конца потока, эскейпа и общее количество
// символов
#define END_OF_STREAM 256
#define ESCAPE 257
#define SYMBOL_COUNT 258

// Количество узлов в дереве (дважды больше, чем количество символов минус один)
#define NODE_TABLE_COUNT ((SYMBOL_COUNT * 2) - 1)

// Индекс корневого узла в дереве
#define ROOT_NODE 0

// Максимальный вес узла
#define MAX_WEIGHT 0x8000

// Булевы значения
#define TRUE 1
#define FALSE 0

// Структура для управления сжатым файлом
typedef struct bit_file {
  FILE *file; // Указатель на файловую переменную
  unsigned char mask; // Маска для битовых операций
  int rack; // Регистр, который накапливает биты перед записью в файл
  int pacifier_counter; // Счетчик для отслеживания прогресса при записи
} COMPRESSED_FILE;

typedef struct tree
{
    int leaf[ SYMBOL_COUNT ];  // Массив, отслеживающий, является ли узел листовым (1) или нет (0) для каждого символа
    int next_free_node;        // Индекс следующего свободного узла в массиве nodes
    
    struct node
    {
        unsigned int weight;   // Вес узла, используется для построения адаптивного дерева Хаффмана
        int parent;             // Индекс родительского узла в массиве nodes
        int child_is_leaf;      // Флаг, указывающий, является ли дочерний узел листовым (1) или нет (0)
        int child;              // Индекс дочернего узла в массиве nodes
    }
    nodes[ NODE_TABLE_COUNT ]; // Массив структур node, представляющих узлы дерева
    
} TREE;
// Внешние объявления глобальных переменных
extern char *Usage;
extern char *CompressionName;

// Прототипы функций
void usage_exit(char *prog_name); // Завершает программу с сообщением об использовании
void print_ratios(char *input, char *output); // Выводит соотношение размеров входного и выходного файлов
long file_size(char *name); // Возвращает размер указанного файла
void fatal_error(char *fmt, ...); // Выводит сообщение о фатальной ошибке и завершает программу

// Прототипы функций для работы с файлами
COMPRESSED_FILE *OpenInputCompressedFile(char *name); // Открывает входной сжатый файл
COMPRESSED_FILE *OpenOutputCompressedFile(char *name); // Открывает выходной сжатый файл

// Прототипы функций для операций с битами в COMPRESSED_FILE
void OutputBit(COMPRESSED_FILE *bit_file, int bit); // Записывает один бит в сжатый файл
void OutputBits(COMPRESSED_FILE *bit_file, unsigned long code, int count); // Записывает указанное количество бит в сжатый файл
int InputBit(COMPRESSED_FILE *bit_file); // Считывает один бит из сжатого файла
unsigned long InputBits(COMPRESSED_FILE *bit_file, int bit_count); // Считывает указанное количество бит из сжатого файла
void CloseInputCompressedFile(COMPRESSED_FILE *bit_file); // Закрывает входной сжатый файл
void CloseOutputCompressedFile(COMPRESSED_FILE *bit_file); // Закрывает выходной сжатый файл

// Прототипы функций для сжатия и декомпрессии файлов
void CompressFile(FILE *input, COMPRESSED_FILE *output); // Сжимает указанный входной файл и записывает в выходной сжатый файл
void ExpandFile(COMPRESSED_FILE *input, FILE *output); // Декомпрессирует указанный входной сжатый файл и записывает в выходной файл

// Прототипы функций для работы с деревом
void InitializeTree(TREE *tree); // Инициализирует структуру дерева
void EncodeSymbol(TREE *tree, unsigned int c, COMPRESSED_FILE *output); // Кодирует символ с использованием структуры дерева
int DecodeSymbol(TREE *tree, COMPRESSED_FILE *input); // Декодирует символ с использованием структуры дерева
void UpdateModel(TREE *tree, int c); // Обновляет модель на основе входного символа
void RebuildTree(TREE *tree); // Перестраивает структуру дерева
void swap_nodes(TREE *tree, int i, int j); // Обменивает местами два узла в структуре дерева
void add_new_node(TREE *tree, int c); // Добавляет новый узел в структуру дерева
