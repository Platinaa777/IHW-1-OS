## Мирошниченко Денис Александрович БПИ 225

# Вариант 3 (5 баллов, см. дальше для других баллов)

## *Cхема подробная для всех процессов и каналов:*

**Этапы:**

**Создание именованных каналов:**

- Создаются два именованных канала с помощью функции mknod. Названия каналов задаются как pipe1 и pipe2.
- Проверяется успешное создание каналов.

---

**Инициализация каналов:**

- Открываются именованные каналы в режимах чтения и записи с помощью функции open. Для каждого канала создаются два файловых дескриптора.
- Проверяется успешное открытие каналов.

---

**Программа состоит из трех основных функций, выполняемых каждым из трех процессов:**

- Первый процесс (readFromFile) открывает входной файл для чтения, считывает данные и записывает их в первый именованный канал.

- Второй процесс (DoMainWork) читает данные из первого канала, обрабатывает их в соответствии с условием задачи (подсчет последовательности), и записывает результат во второй именованный канал.

- Третий процесс (writeToOutputFile) читает данные из второго канала и записывает их в выходной файл.

Для обеспечения взаимодействия между процессами используются именованные каналы, создаваемые с помощью функции mknod().

---

**Более подробное описание:**

- Создание именованных каналов (pipe1 и pipe2) с помощью функции mknod().
- Создание трех процессов (pid1, pid2, pid3) с помощью fork().
- Первый процесс (pid1) открывает входной файл, читает данные и записывает их в первый именованный канал (pipe1).
- Второй процесс (pid2) читает данные из первого канала (pipe1), обрабатывает их и записывает результат во второй именованный канал (pipe2).
- Третий процесс (pid3) читает данные из второго канала (pipe2) и записывает их в выходной файл.
- Все процессы завершают свою работу и освобождают ресурсы.
- Главная программа ожидает завершения всех процессов, после чего закрывает все открытые каналы и удаляет их.

---

## Формат вводна данных в cmd:

[ *input-file output-file n* ]

---

Ввод и вывод данных при работе с файлами осуществлять через системные вызовы операционной системы read и write.

---

Размеры буферов для хранения вводимых данных и результатов обработки должны быть равны 5000 байт.

```
#define BUFFER_SIZE 5000
```

## Отчет по тестам:

**Справка:**

- Первое название файл, с которого берется информация

- Второй файл, куда записывается результат


---

- **1 command:** ./a.out ./tests/test1.txt ./out/out1.txt 4
    
    *src = test1.txt*

    *out = out1.txt*

    *res=5432*

---

- **2 command:** ./a.out ./tests/test2.txt ./out/out2.txt 3
    
    *src = test2.txt*

    *out = out2.txt*

    *res=fdc*

---

- **3 command:** ./a.out ./tests/test3.txt ./out/out3.txt 2
    
    *src = test3.txt*

    *out = out3.txt*

    *res=he* 

---

- **4 command:** ./a.out ./tests/test4.txt ./out/out4.txt 3
    
    *src = test4.txt*

    *out = out4.txt*

    *res=''*

---

- **5 command:** ./a.out ./tests/test5.txt ./out/out5.txt 2
    
    *src = test5.txt*

    *out = out5.txt*

    *res=''*