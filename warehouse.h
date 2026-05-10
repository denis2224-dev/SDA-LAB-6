#ifndef WAREHOUSE_H
#define WAREHOUSE_H

#include <stddef.h>
#include <stdio.h>

#define MAX_TEXT 50

typedef enum {
    UNIT_TONS = 1,
    UNIT_KG = 2,
    UNIT_GRAMS = 3
} UnitType;

typedef struct {
    int day;
    int month;
    int year;
} Date;

typedef struct {
    char owner_name[MAX_TEXT];
    char owner_surname[MAX_TEXT];
    char product_name[MAX_TEXT];
    char manufacturer[MAX_TEXT];
    Date contract_date;
    double wholesale_price;
    double unit_price;
    double quantity;
    UnitType unit;
} WarehouseRecord;

int validate_date(const Date *date);
int is_valid_unit(int unit_raw);
const char *unit_type_to_string(UnitType unit);
double convert_to_kg(double quantity, UnitType unit);
double warehouse_total_value(const WarehouseRecord *record);
int warehouse_auto_priority(const WarehouseRecord *record);
void print_warehouse_record(FILE *stream, const WarehouseRecord *record, int index, int priority);

#endif
