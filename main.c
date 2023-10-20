#include "header.c"
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
  // Инициализация указателей на файлы
  COMPRESSED_FILE *output;
  FILE *input;

  // Отключение буферизации стандартного вывода
  setbuf(stdout, NULL);

  // Проверка наличия необходимого числа аргументов командной строки
  if (argc < 3)
    usage_exit(argv[0]);

  // Открытие входного файла для чтения в бинарном режиме
  input = fopen(argv[1], "rb");
  if (input == NULL)
    fatal_error("Failed to open the input file: %s\n",
                argv[1]); // Не удалось открыть входной файл

  // Открытие выходного сжатого файла для записи
  output = OpenOutputCompressedFile(argv[2]);
  if (output == NULL)
    fatal_error("Failed to open output compressed file: %s\n",
                argv[2]); // Не удалось открыть выходной сжатый файл

  // Вывод информации о текущей операции
  printf("\nfile compression %s в %s,\n", argv[1], argv[2]); // Сжатие файла
  printf("Compression method used: %s\n",
         CompressionName); // Используемый метод сжатия

  // Выполнение сжатия файла
  CompressFile(input, output);

  // Закрытие выходного сжатого файла
  CloseOutputCompressedFile(output);

  // Закрытие входного файла
  fclose(input);

  // Вывод статистики о размерах файлов и степени сжатия
  print_ratios(argv[1], argv[2]);

  // Возврат успешного завершения программы
  return 0;
}
// Функция для отображения информации о использовании и завершения программы
void usage_exit(char *prog_name) {
  char *short_name; // Указатель на короткое имя программы
  char *extension; // Указатель на расширение файла программы

  // Попытка найти последний символ обратного слеша в имени программы
  short_name = strrchr(prog_name, '\\');
  // Если не найден, попытка найти последний символ прямого слеша
  if (short_name == NULL)
    short_name = strrchr(prog_name, '/');
  // Если все еще не найден, попытка найти последний символ двоеточия
  if (short_name == NULL)
    short_name = strrchr(prog_name, ':');
  // Если хотя бы один из вышеуказанных символов найден, переместите указатель
  // на следующий символ
  if (short_name != NULL)
    short_name++;
  // Если ни один из разделителей не был найден, используйте полное имя
  // программы
  else
    short_name = prog_name;

  // Попытка найти последнюю точку в коротком имени (чтобы определить расширение
  // файла)
  extension = strrchr(short_name, '.');
  // Если точка найдена, укоротите короткое имя до этой позиции, чтобы удалить
  // расширение
  if (extension != NULL)
    *extension = '\0';

  // Вывод информации о использовании с измененным коротким именем
  printf("\nUtilisation info:  %s %s\n", short_name, Usage);

  // Завершение программы
  exit(0);
}

// Если SEEK_END не определено, определяем его как 2
#ifndef SEEK_END
#define SEEK_END 2
#endif

// Функция для получения размера файла по его имени
long file_size(char *name) {
  // Переменная для хранения конечной позиции в файле
  long eof_ftell;
  // Указатель на файл
  FILE *file;

  // Открываем файл в режиме чтения
  file = fopen(name, "r");
  // Если не удалось открыть файл, возвращаем 0
  if (file == NULL)
    return (0L);

  // Переходим в конец файла
  fseek(file, 0L, SEEK_END);
  // Получаем текущую позицию (конечную) в файле
  eof_ftell = ftell(file);

  // Закрываем файл
  fclose(file);

  // Возвращаем размер файла (конечную позицию)
  return (eof_ftell);
}


// Функция для вывода статистики о размерах файлов и степени сжатия
void print_ratios(char *input, char *output) {
  // Переменные для хранения размеров входного и выходного файлов, а также степени сжатия
  long input_size;
  long output_size;
  int ratio;

  // Получаем размер входного файла
  input_size = file_size(input);
  // Если размер входного файла равен 0, устанавливаем размер в 1 (избегаем деления на 0)
  if (input_size == 0)
    input_size = 1;

  // Получаем размер выходного файла
  output_size = file_size(output);

  // Вычисляем степень сжатия в процентах
  ratio = 100 - (int)(output_size * 100L / input_size);

  // Выводим информацию о размерах файлов и степени сжатия
  printf("\nРазмер входного файла:\t%ld байт\n", input_size);
  printf("Размер выходного файла:\t%ld байт\n", output_size);
  if (output_size == 0)
    output_size = 1;
  printf("Степень сжатия:\t\t%d%%\n", ratio);
}

void fatal_error(char *fmt, ...) {
  va_list argptr;

  va_start(argptr, fmt);
  printf("” в «м п (r)иЁЎЄ : ");
  vprintf(fmt, argptr);
  va_end(argptr);
  exit(-1);
}

#define PACIFIER_COUNT 2047

COMPRESSED_FILE *OpenOutputCompressedFile(char *name) {
  COMPRESSED_FILE *compressed_file;

  compressed_file = (COMPRESSED_FILE *)calloc(1, sizeof(COMPRESSED_FILE));
  if (compressed_file == NULL)
    return (compressed_file);
  compressed_file->file = fopen(name, "wb");
  compressed_file->rack = 0;
  compressed_file->mask = 0x80;
  compressed_file->pacifier_counter = 0;
  return (compressed_file);
}

COMPRESSED_FILE *OpenInputCompressedFile(char *name) {
  COMPRESSED_FILE *compressed_file;

  compressed_file = (COMPRESSED_FILE *)calloc(1, sizeof(COMPRESSED_FILE));
  if (compressed_file == NULL)
    return (compressed_file);
  compressed_file->file = fopen(name, "rb");
  compressed_file->rack = 0;
  compressed_file->mask = 0x80;
  compressed_file->pacifier_counter = 0;
  return (compressed_file);
}

void CloseOutputCompressedFile(COMPRESSED_FILE *compressed_file) {
  if (compressed_file->mask != 0x80)
    if (putc(compressed_file->rack, compressed_file->file) !=
        compressed_file->rack)
      fatal_error("” в «м п (r)иЁЎЄ  ЇаЁ Ї(r)ЇлвЄҐ § Єалвм б¦ вл(c) д (c)«!\n");
  fclose(compressed_file->file);
  free((char *)compressed_file);
}

void CloseInputCompressedFile(COMPRESSED_FILE *compressed_file) {
  fclose(compressed_file->file);
  free((char *)compressed_file);
}

void OutputBit(COMPRESSED_FILE *compressed_file, int bit) {
  if (bit)
    compressed_file->rack |= compressed_file->mask;
  compressed_file->mask >>= 1;
  if (compressed_file->mask == 0) {
    if (putc(compressed_file->rack, compressed_file->file) !=
        compressed_file->rack)
      fatal_error("” в «м п (r)иЁЎЄ  ў Їа(r)жҐ¤гаҐ OutputBit!\n");
    else if ((compressed_file->pacifier_counter++ & PACIFIER_COUNT) == 0)
      putc('.', stdout);
    compressed_file->rack = 0;
    compressed_file->mask = 0x80;
  }
}

void OutputBits(COMPRESSED_FILE *compressed_file, unsigned long code,
                int count) {
  unsigned long mask;

  mask = 1L << (count - 1);
  while (mask != 0) {
    if (mask & code)
      compressed_file->rack |= compressed_file->mask;
    compressed_file->mask >>= 1;
    if (compressed_file->mask == 0) {
      if (putc(compressed_file->rack, compressed_file->file) !=
          compressed_file->rack)
        fatal_error("” в «м п (r)иЁЎЄ  ў Їа(r)жҐ¤гаҐ OutputBits!\n");
      else if ((compressed_file->pacifier_counter++ & PACIFIER_COUNT) == 0)
        putc('.', stdout);
      compressed_file->rack = 0;
      compressed_file->mask = 0x80;
    }
    mask >>= 1;
  }
}

int InputBit(COMPRESSED_FILE *compressed_file) {
  int value;

  if (compressed_file->mask == 0x80) {
    compressed_file->rack = getc(compressed_file->file);
    if (compressed_file->rack == EOF)
      fatal_error("” в «м п (r)иЁЎЄ  ў Їа(r)жҐ¤гаҐ InputBit!\n");
    if ((compressed_file->pacifier_counter++ & PACIFIER_COUNT) == 0)
      putc('.', stdout);
  }
  value = compressed_file->rack & compressed_file->mask;
  compressed_file->mask >>= 1;
  if (compressed_file->mask == 0)
    compressed_file->mask = 0x80;
  return (value ? 1 : 0);
}

unsigned long InputBits(COMPRESSED_FILE *compressed_file, int bit_count) {
  unsigned long mask;
  unsigned long return_value;

  mask = 1L << (bit_count - 1);
  return_value = 0;
  while (mask != 0) {
    if (compressed_file->mask == 0x80) {
      compressed_file->rack = getc(compressed_file->file);
      if (compressed_file->rack == EOF)
        fatal_error("” в «м п (r)иЁЎЄ  ў Їа(r)жҐ¤гаҐ InputBits!\n");
      if ((compressed_file->pacifier_counter++ & PACIFIER_COUNT) == 0)
        putc('.', stdout);
    }
    if (compressed_file->rack & compressed_file->mask)
      return_value |= mask;
    mask >>= 1;
    compressed_file->mask >>= 1;
    if (compressed_file->mask == 0)
      compressed_file->mask = 0x80;
  }
  return (return_value);
}

char *CompressionName = "Ђ¤ ЇвЁў(r)Ґ Љ(r)¤Ёа(r)ў ЁҐ • дд¬Ґ ";
char *Usage = "зв(r)_б¦Ё¬ вм Єг¤ _б¦Ё¬ вм";

TREE Tree;

void CompressFile(FILE *input, COMPRESSED_FILE *output) {
  int c;

  InitializeTree(&Tree);
  while ((c = getc(input)) != EOF) {
    EncodeSymbol(&Tree, c, output);
    UpdateModel(&Tree, c);
  }
  EncodeSymbol(&Tree, END_OF_STREAM, output);
}

void ExpandFile(COMPRESSED_FILE *input, FILE *output) {
  int c;

  InitializeTree(&Tree);
  while ((c = DecodeSymbol(&Tree, input)) != END_OF_STREAM) {
    if (putc(c, output) == EOF)
      fatal_error("ЌҐ ¬(r)Јг ЇЁб вм ў ўле(r)¤(r)(c) д (c)« ЇаЁ а бЇ Є(r)ўЄҐ");
    UpdateModel(&Tree, c);
  }
}

void InitializeTree(TREE *tree) {
  int i;

  tree->nodes[ROOT_NODE].child = ROOT_NODE + 1;
  tree->nodes[ROOT_NODE].child_is_leaf = FALSE;
  tree->nodes[ROOT_NODE].weight = 2;
  tree->nodes[ROOT_NODE].parent = -1;

  tree->nodes[ROOT_NODE + 1].child = END_OF_STREAM;
  tree->nodes[ROOT_NODE + 1].child_is_leaf = TRUE;
  tree->nodes[ROOT_NODE + 1].weight = 1;
  tree->nodes[ROOT_NODE + 1].parent = ROOT_NODE;
  tree->leaf[END_OF_STREAM] = ROOT_NODE + 1;

  tree->nodes[ROOT_NODE + 2].child = ESCAPE;
  tree->nodes[ROOT_NODE + 2].child_is_leaf = TRUE;
  tree->nodes[ROOT_NODE + 2].weight = 1;
  tree->nodes[ROOT_NODE + 2].parent = ROOT_NODE;
  tree->leaf[ESCAPE] = ROOT_NODE + 2;

  tree->next_free_node = ROOT_NODE + 3;

  for (i = 0; i < END_OF_STREAM; i++)
    tree->leaf[i] = -1;
}

void EncodeSymbol(TREE *tree, unsigned int c, COMPRESSED_FILE *output) {
  unsigned long code;
  unsigned long current_bit;
  int code_size;
  int current_node;

  code = 0;
  current_bit = 1;
  code_size = 0;
  current_node = tree->leaf[c];
  if (current_node == -1)
    current_node = tree->leaf[ESCAPE];
  while (current_node != ROOT_NODE) {
    if ((current_node & 1) == 0)
      code |= current_bit;
    current_bit <<= 1;
    code_size++;
    current_node = tree->nodes[current_node].parent;
  }
  OutputBits(output, code, code_size);
  if (tree->leaf[c] == -1) {
    OutputBits(output, (unsigned long)c, 8);
    add_new_node(tree, c);
  }
}

int DecodeSymbol(TREE *tree, COMPRESSED_FILE *input) {
  int current_node;
  int c;

  current_node = ROOT_NODE;
  while (!tree->nodes[current_node].child_is_leaf) {
    current_node = tree->nodes[current_node].child;
    current_node += InputBit(input);
  }
  c = tree->nodes[current_node].child;
  if (c == ESCAPE) {
    c = (int)InputBits(input, 8);
    add_new_node(tree, c);
  }
  return (c);
}

void UpdateModel(TREE *tree, int c) {
  int current_node;
  int new_node;

  if (tree->nodes[ROOT_NODE].weight == MAX_WEIGHT)
    RebuildTree(tree);
  current_node = tree->leaf[c];
  while (current_node != -1) {
    tree->nodes[current_node].weight++;
    for (new_node = current_node; new_node > ROOT_NODE; new_node--)
      if (tree->nodes[new_node - 1].weight >= tree->nodes[current_node].weight)
        break;
    if (current_node != new_node) {
      swap_nodes(tree, current_node, new_node);
      current_node = new_node;
    }
    current_node = tree->nodes[current_node].parent;
  }
}

void RebuildTree(TREE *tree) {
  int i;
  int j;
  int k;
  unsigned int weight;

  printf("R");
  j = tree->next_free_node - 1;
  for (i = j; i >= ROOT_NODE; i--) {
    if (tree->nodes[i].child_is_leaf) {
      tree->nodes[j] = tree->nodes[i];
      tree->nodes[j].weight = (tree->nodes[j].weight + 1) / 2;
      j--;
    }
  }

  for (i = tree->next_free_node - 2; j >= ROOT_NODE; i -= 2, j--) {
    k = i + 1;
    tree->nodes[j].weight = tree->nodes[i].weight + tree->nodes[k].weight;
    weight = tree->nodes[j].weight;
    tree->nodes[j].child_is_leaf = FALSE;
    for (k = j + 1; weight < tree->nodes[k].weight; k++)
      ;
    k--;
    memmove(&tree->nodes[j], &tree->nodes[j + 1],
            (k - j) * sizeof(struct node));
    tree->nodes[k].weight = weight;
    tree->nodes[k].child = i;
    tree->nodes[k].child_is_leaf = FALSE;
  }

  for (i = tree->next_free_node - 1; i >= ROOT_NODE; i--) {
    if (tree->nodes[i].child_is_leaf) {
      k = tree->nodes[i].child;
      tree->leaf[k] = i;
    } else {
      k = tree->nodes[i].child;
      tree->nodes[k].parent = tree->nodes[k + 1].parent = i;
    }
  }
}

void swap_nodes(TREE *tree, int i, int j) {
  struct node temp;

  if (tree->nodes[i].child_is_leaf)
    tree->leaf[tree->nodes[i].child] = j;
  else {
    tree->nodes[tree->nodes[i].child].parent = j;
    tree->nodes[tree->nodes[i].child + 1].parent = j;
  }
  if (tree->nodes[j].child_is_leaf)
    tree->leaf[tree->nodes[j].child] = i;
  else {
    tree->nodes[tree->nodes[j].child].parent = i;
    tree->nodes[tree->nodes[j].child + 1].parent = i;
  }
  temp = tree->nodes[i];
  tree->nodes[i] = tree->nodes[j];
  tree->nodes[i].parent = temp.parent;
  temp.parent = tree->nodes[j].parent;
  tree->nodes[j] = temp;
}

void add_new_node(TREE *tree, int c) {
  int lightest_node;
  int new_node;
  int zero_weight_node;

  lightest_node = tree->next_free_node - 1;
  new_node = tree->next_free_node;
  zero_weight_node = tree->next_free_node + 1;
  tree->next_free_node += 2;

  tree->nodes[new_node] = tree->nodes[lightest_node];
  tree->nodes[new_node].parent = lightest_node;
  tree->leaf[tree->nodes[new_node].child] = new_node;

  tree->nodes[lightest_node].child = new_node;
  tree->nodes[lightest_node].child_is_leaf = FALSE;

  tree->nodes[zero_weight_node].child = c;
  tree->nodes[zero_weight_node].child_is_leaf = TRUE;
  tree->nodes[zero_weight_node].weight = 0;
  tree->nodes[zero_weight_node].parent = lightest_node;
  tree->leaf[c] = zero_weight_node;
}