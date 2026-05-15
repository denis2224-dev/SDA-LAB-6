# Lab 6 - Solving Problems with STACK/QUEUE ADT + FILE + UDT (C)

Procedural C project for Lab 6 using:
- linked-list stack ADT;
- linked-list queue ADTs (Simple, Double Ended, Circular, Priority);
- dynamic memory allocation;
- FILE operations;
- warehouse registry UDT reused from Lab 5.

The warehouse record keeps:
- owner name/surname;
- product name;
- manufacturer;
- contract date (day, month, year);
- wholesale price;
- unit price;
- quantity;
- unit type (`tons`, `kg`, `grams`).

## Relation to Lab 5

This project reuses and extends the Lab 5 warehouse registry idea and FILE workflow (`experiment.txt`, `output.txt`, and copying output to the beginning of experiment file), adapted to queue ADT tasks.

## Project structure

```
LAB6/
  CMakeLists.txt
  README.md
  main.c
  warehouse.h
  warehouse.c
  queue.h
  queue.c
  file_ops.h
  file_ops.c
  data/
    experiment.txt
    output.txt
    test_scenarios_input.txt
```

## Build and run

```bash
mkdir -p build
cd build
cmake ..
cmake --build .
./lab6_queue_adt
```

## Main menu

The first menu chooses the data structure:

1. Stack
2. Queue
0. Exit

After that, each data structure has its own operations menu.

## Stack menu

1. Load records from `experiment.txt`
2. Add record manually
3. Display records
4. Stack operations
5. Search record
6. Delete record
7. Save current stack to `output.txt`
8. Copy `output.txt` to beginning of `experiment.txt`
9. Delete file
0. Back to Stack/Queue selection

Stack operations:
- **push**: insert a record at the top;
- **pop**: remove the top record;
- **peek**: view the top record;
- **display**: traverse from top to bottom.

## Queue menu

1. Load records from `experiment.txt`  
2. Add record manually  
3. Display records  
4. Simple Queue operations  
5. Double Ended Queue operations  
6. Circular Queue operations  
7. Priority Queue operations  
8. Search record  
9. Delete record  
10. Save current queue to `output.txt`  
11. Copy `output.txt` to beginning of `experiment.txt`  
12. Delete file  
0. Back to Stack/Queue selection

## Queue type behavior

- **Simple Queue**: enqueue rear, dequeue front, peek front, traversal display.
- **Double Ended Queue**: insert/delete at both front and rear, forward/backward display.
- **Circular Queue**: linked-list queue where `rear->next` points to `front`.
- **Priority Queue**: higher priority served first; equal priority preserves insertion order.

Priority rule:
`priority = quantity_in_kg * unit_price`  
Manual override is available when inserting.

## FILE operations

- create/open `experiment.txt` and `output.txt`;
- read records and append/save to files in semicolon-separated format;
- load file records into selected queue type;
- save queue state to `output.txt` (append or rewrite);
- copy `output.txt` to beginning of `experiment.txt` while preserving old content;
- reopen existing file for reading;
- delete selected file with confirmation.

## Search and delete

Search supports:
- by position;
- by product name;
- by owner surname;
- by manufacturer;
- by contract year;
- by unit price interval.

Delete supports:
- by position;
- by product name;
- by owner surname;
- delete-all matching condition with confirmation.

## Data format

Semicolon-separated, one record per line:

`owner_name;owner_surname;product_name;manufacturer;day;month;year;wholesale_price;unit_price;quantity;unit`

Example:

`Ion;Popescu;Rice;AgroMold;12;4;2026;20.50;25.00;120;2`

## Sample input

Use interactive keyboard input from menu, or run the scripted smoke scenario with:

```bash
./build/lab6_queue_adt < data/test_scenarios_input.txt
```

## Code documentation notes

- The source is documented with inline C comments (`// Comment` / `// Coment` style).
- Comments explain logic decisions and data-flow steps, not obvious syntax.
