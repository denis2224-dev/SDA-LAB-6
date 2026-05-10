#ifndef WAREHOUSE_H
#define WAREHOUSE_H

#include <stddef.h>
#include <stdio.h>

#define MAX_TEXT 50

// Quantity unit used in warehouse records.
typedef enum {
    UNIT_TONS = 1,
    UNIT_KG = 2,
    UNIT_GRAMS = 3
} UnitType;

// Contract date (numeric day/month/year format).
typedef struct {
    int day;
    int month;
    int year;
} Date;

// Lab 5 warehouse registry UDT adapted for Lab 6 queues.
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

// Returns 1 if date is valid, otherwise 0.
int validate_date(const Date *date);
// Returns 1 when unit_raw is a valid UnitType value.
int is_valid_unit(int unit_raw);
// Returns a readable unit string.
const char *unit_type_to_string(UnitType unit);
// Converts quantity from selected unit to kilograms.
double convert_to_kg(double quantity, UnitType unit);
// Computes total value as quantity_in_kg * unit_price.
double warehouse_total_value(const WarehouseRecord *record);
// Computes automatic priority from total value.
int warehouse_auto_priority(const WarehouseRecord *record);
// Prints one formatted record line including index, total value and priority.
void print_warehouse_record(FILE *stream, const WarehouseRecord *record, int index, int priority);

#endif
