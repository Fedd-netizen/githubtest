k#include <stdio.h>
#include <stdlib.h>
#include <string.h>
haloooo
bsdchdshc

#define MAX_LINE_LENGTH 1024
#define MAX_ENTRIES 100

typedef struct {
    char date[11];
    char patient_id[20];
    char diagnosis[50];
    char tindakan[50];
    char kontrol[20];
    float cost;
} Entry;

typedef struct {
    char period[8]; // YYYY-MM or YYYY
    float total_income;
    char patients[MAX_ENTRIES][MAX_LINE_LENGTH];
    int patient_count;
} PeriodData;

void remove_commas(char* str) {
    char *src, *dst;
    for (src = dst = str; *src != '\0'; src++) {
        if (*src != '.') {
            *dst++ = *src;
        }
    }
    *dst = '\0';
}

void normalize_date(const char* date, char* normalized_date, int by_month) {
    int day, year;
    char month[4];
    char month_map[][4] = {"Jan", "Feb", "Mar", "Apr", "Mei", "Jun", "Jul", "Agu", "Sep", "Okt", "Nov", "Des"};
    int month_number = 0;

    if (strchr(date, '-') != NULL) {
        // Format: dd-MMM-yy
        sscanf(date, "%d-%3s-%d", &day, month, &year);
        if (year < 50) year += 2000; else year += 1900; // Handle years like '23 as 2023
        for (int i = 0; i < 12; i++) {
            if (strncmp(month, month_map[i], 3) == 0) {
                month_number = i + 1;
                break;
            }
        }
    } else {
        // Format: dd month yyyy
        char month_full[10];
        sscanf(date, "%d %9s %d", &day, month_full, &year);
        for (int i = 0; i < 12; i++) {
            if (strncasecmp(month_full, month_map[i], 3) == 0) {
                month_number = i + 1;
                break;
            }
        }
    }

    if (by_month) {
        sprintf(normalized_date, "%04d-%02d", year, month_number);
    } else {
        sprintf(normalized_date, "%04d", year);
    }
}

void add_entry(PeriodData* period_data, const Entry* entry) {
    period_data->total_income += entry->cost;
    char patient_info[MAX_LINE_LENGTH];
    sprintf(patient_info, "%s, %s, %s, %s, %.2f", entry->patient_id, entry->diagnosis, entry->tindakan, entry->kontrol, entry->cost);

    int patient_exists = 0;
    for (int i = 0; i < period_data->patient_count; i++) {
        if (strcmp(period_data->patients[i], patient_info) == 0) {
            patient_exists = 1;
            break;
        }
    }
    if (!patient_exists) {
        strcpy(period_data->patients[period_data->patient_count++], patient_info);
    }
}

int compare_periods(const void* a, const void* b) {
    return strcmp(((PeriodData*)b)->period, ((PeriodData*)a)->period);
}

int main() {
    FILE *file = fopen("data.csv", "r");
    if (file == NULL) {
        perror("Unable to open file");
        return 1;
    }

    Entry entries[MAX_ENTRIES];
    int entry_count = 0;

    char line[MAX_LINE_LENGTH];
    char *fields[MAX_ENTRIES];
    int index = 0;

    // Skip the header line
    if (fgets(line, sizeof(line), file)) {
        // Read each line of the file
        while (fgets(line, sizeof(line), file)) {
            int field_index = 0;
            char *token = strtok(line, ",");

            // Parse each field in the line
            while (token != NULL) {
                fields[field_index++] = token;
                token = strtok(NULL, ",");
            }

            // Assuming "Tanggal" is in the second column, "ID Pasien" is in the third column, "Diagnosis" in the fourth column, "Tindakan" in the fifth column, "Kontrol" in the sixth column, and "Biaya (Rp)" is in the eighth column
            if (field_index >= 8) {
                // Remove newline character from the last field
                fields[7][strcspn(fields[7], "\n")] = '\0';

                remove_commas(fields[7]);
                entries[entry_count].cost = atof(fields[7]);
                strcpy(entries[entry_count].date, fields[1]);
                strcpy(entries[entry_count].patient_id, fields[2]);
                strcpy(entries[entry_count].diagnosis, fields[3]);
                strcpy(entries[entry_count].tindakan, fields[4]);
                strcpy(entries[entry_count].kontrol, fields[5]);
                entry_count++;
            }
        }
    }

    fclose(file);

    PeriodData month_data[MAX_ENTRIES];
    PeriodData year_data[MAX_ENTRIES];
    int month_count = 0;
    int year_count = 0;

    for (int i = 0; i < entry_count; i++) {
        char normalized_date[11];

        // Process monthly data
        normalize_date(entries[i].date, normalized_date, 1);
        int found = 0;
        for (int j = 0; j < month_count; j++) {
            if (strcmp(month_data[j].period, normalized_date) == 0) {
                add_entry(&month_data[j], &entries[i]);
                found = 1;
                break;
            }
        }
        if (!found) {
            strcpy(month_data[month_count].period, normalized_date);
            month_data[month_count].total_income = 0;
            month_data[month_count].patient_count = 0;
            add_entry(&month_data[month_count], &entries[i]);
            month_count++;
        }

        // Process yearly data
        normalize_date(entries[i].date, normalized_date, 0);
        found = 0;
        for (int j = 0; j < year_count; j++) {
            if (strcmp(year_data[j].period, normalized_date) == 0) {
                add_entry(&year_data[j], &entries[i]);
                found = 1;
                break;
            }
        }
        if (!found) {
            strcpy(year_data[year_count].period, normalized_date);
            year_data[year_count].total_income = 0;
            year_data[year_count].patient_count = 0;
            add_entry(&year_data[year_count], &entries[i]);
            year_count++;
        }
    }

    // Sort the data in chronological order (latest to oldest)
    qsort(month_data, month_count, sizeof(PeriodData), compare_periods);
    qsort(year_data, year_count, sizeof(PeriodData), compare_periods);

    char option[10];
    printf("Enter 'month' to see total income per month or 'year' to see total income per year: ");
    scanf("%s", option);

    if (strcmp(option, "month") == 0) {
        for (int i = 0; i < month_count; i++) {
            printf("Period: %s\n", month_data[i].period);
            printf("Total Income: %.2f\n", month_data[i].total_income);
            for (int j = 0; j < month_data[i].patient_count; j++) {
                printf("%s\n", month_data[i].patients[j]);
            }
            printf("\n");
        }
    } else if (strcmp(option, "year") == 0) {
        float total_income_all_years = 0;
        for (int i = 0; i < year_count; i++) {
            total_income_all_years += year_data[i].total_income;
        }
        float average_income_per_year = total_income_all_years / year_count;

        for (int i = 0; i < year_count; i++) {
            printf("Period: %s\n", year_data[i].period);
            printf("Total Income: %.2f\n", year_data[i].total_income);
            for (int j = 0; j < year_data[i].patient_count; j++) {
                printf("%s\n", year_data[i].patients[j]);
            }
            printf("\n");
        }
        printf("Average Yearly Income: %.2f\n", average_income_per_year);
    } else {
        printf("Invalid option.\n");
    }

    return 0;
}
