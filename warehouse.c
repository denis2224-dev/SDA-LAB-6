#include "warehouse.h"

#include <string.h>

int validate_date(const Date *date) {
    static const int days_per_month[12] = {
        31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };
    int max_day;
    int is_leap;

    if (date == NULL) {
        return 0;
    }
    if (date->year < 1900 || date->year > 2100) {
        return 0;
    }
    if (date->month < 1 || date->month > 12) {
        return 0;
    }

    max_day = days_per_month[date->month - 1];
    is_leap = (date->year % 4 == 0 && date->year % 100 != 0) || (date->year % 400 == 0);
    if (date->month == 2 && is_leap) {
        max_day = 29;
    }

    return date->day >= 1 && date->day <= max_day;
}

int is_valid_unit(int unit_raw) {
    return unit_raw >= UNIT_TONS && unit_raw <= UNIT_GRAMS;
}

const char *unit_type_to_string(UnitType unit) {
    switch (unit) {
        case UNIT_TONS:
            return "tons";
        case UNIT_KG:
            return "kg";
        case UNIT_GRAMS:
            return "grams";
        default:
            return "unknown";
    }
}

double convert_to_kg(double quantity, UnitType unit) {
    if (unit == UNIT_TONS) {
        return quantity * 1000.0;
    }
    if (unit == UNIT_GRAMS) {
        return quantity / 1000.0;
    }
    return quantity;
}

double warehouse_total_value(const WarehouseRecord *record) {
    double quantity_kg;

    if (record == NULL) {
        return 0.0;
    }

    quantity_kg = convert_to_kg(record->quantity, record->unit);
    return quantity_kg * record->unit_price;
}

int warehouse_auto_priority(const WarehouseRecord *record) {
    double total_value;

    if (record == NULL) {
        return 0;
    }

    total_value = warehouse_total_value(record);
    if (total_value < 0.0) {
        return 0;
    }
    if (total_value > 2147483647.0) {
        return 2147483647;
    }

    return (int)(total_value + 0.5);
}

void print_warehouse_record(FILE *stream, const WarehouseRecord *record, int index, int priority) {
    double total_value;

    if (stream == NULL || record == NULL) {
        return;
    }

    total_value = warehouse_total_value(record);
    fprintf(stream,
            "%d | %s %s | %s | %s | %02d-%02d-%04d | wholesale=%.2f | unit=%.2f | %.2f %s | total=%.2f | priority=%d\n",
            index,
            record->owner_name,
            record->owner_surname,
            record->product_name,
            record->manufacturer,
            record->contract_date.day,
            record->contract_date.month,
            record->contract_date.year,
            record->wholesale_price,
            record->unit_price,
            record->quantity,
            unit_type_to_string(record->unit),
            total_value,
            priority);
}
