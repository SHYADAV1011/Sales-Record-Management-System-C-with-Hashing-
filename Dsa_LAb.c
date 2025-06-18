#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define TABLE_SIZE 1000
#define MAX_RECORDS 1000
#define DATE_LENGTH 11
#define TIME_LENGTH 9
#define STR_LENGTH 30

typedef struct {
    char order_date[DATE_LENGTH];
    char time[TIME_LENGTH];
    float aging;
    int customer_id;
    char gender[STR_LENGTH];
    char device_type[STR_LENGTH];
    char customer_login_type[STR_LENGTH];
    char product_category[STR_LENGTH];
    char product[STR_LENGTH];
    float sales;
    int quantity;
    float discount;
    float profit;
    float shipping_cost;
    char order_priority[STR_LENGTH];
    char payment_method[STR_LENGTH];
} SalesRecord;

typedef struct HashNode {
    SalesRecord record;
    struct HashNode* next;
} HashNode;

typedef struct {
    HashNode* table[TABLE_SIZE];
    int count;
} SalesDirectory;

// Function prototypes
void initializeDirectory(SalesDirectory *dir);
unsigned int hash(int customer_id);
int insertRecord(SalesDirectory *dir, SalesRecord record);
void displayRecord(const SalesRecord *record);
void displayAllRecords(const SalesDirectory *dir);
HashNode* searchRecord(const SalesDirectory *dir, int customer_id);
int deleteRecord(SalesDirectory *dir, int customer_id);
int modifyRecord(SalesDirectory *dir, int customer_id);
int saveToFile(const SalesDirectory *dir, const char *filename);
int loadFromFile(SalesDirectory *dir, const char *filename);
void displayMenu();
void addSampleData(SalesDirectory *dir);
void clearScreen();
void waitForEnter();
int validateDate(const char *date);
int validateTime(const char *time);
int validateCustomerID(const SalesDirectory *dir, int id, HashNode* current_node);
void getStringInput(const char *prompt, char *buffer, int buffer_size);
float getFloatInput(const char *prompt, float min, float max);
int getIntInput(const char *prompt, int min, int max);
void freeHashTable(SalesDirectory *dir);

int main() {
    SalesDirectory directory;
    initializeDirectory(&directory);
    const char *filename = "sales_data.dat";
    
    if (!loadFromFile(&directory, filename)) {
        addSampleData(&directory);
        printf("Sample data loaded (%d records).\n", directory.count);
    } else {
        printf("Existing data loaded (%d records).\n", directory.count);
    }
    
    waitForEnter();
    
    int choice, customer_id, result;
    SalesRecord newRecord;
    
    do {
        clearScreen();
        displayMenu();
        printf("\nEnter your choice: ");
        choice = getIntInput("", 1, 6);
        
        switch (choice) {
            case 1: // Insert new record
                printf("\n--- Add New Sales Record ---\n");
                
                // Get and validate date
                do {
                    getStringInput("Order Date (YYYY-MM-DD): ", newRecord.order_date, DATE_LENGTH);
                } while (!validateDate(newRecord.order_date));
                
                // Get and validate time
                do {
                    getStringInput("Time (HH:MM:SS): ", newRecord.time, TIME_LENGTH);
                } while (!validateTime(newRecord.time));
                
                newRecord.aging = getFloatInput("Aging: ", 0, 100);
                
                // Get and validate customer ID
                do {
                    newRecord.customer_id = getIntInput("Customer ID: ", 1, 999999);
                    result = validateCustomerID(&directory, newRecord.customer_id, NULL);
                    if (result == -1) printf("Invalid ID format. ");
                    else if (result == -2) printf("ID already exists. ");
                } while (result != 0);
                
                getStringInput("Gender: ", newRecord.gender, STR_LENGTH);
                getStringInput("Device Type: ", newRecord.device_type, STR_LENGTH);
                getStringInput("Customer Login Type: ", newRecord.customer_login_type, STR_LENGTH);
                getStringInput("Product Category: ", newRecord.product_category, STR_LENGTH);
                getStringInput("Product: ", newRecord.product, STR_LENGTH);
                
                newRecord.sales = getFloatInput("Sales: ", 0, 1000000);
                newRecord.quantity = getIntInput("Quantity: ", 1, 1000);
                
                // Validate discount between 0 and 1
                do {
                    newRecord.discount = getFloatInput("Discount (0-1): ", 0, 1);
                } while (newRecord.discount < 0 || newRecord.discount > 1);
                
                newRecord.profit = getFloatInput("Profit: ", -1000000, 1000000);
                newRecord.shipping_cost = getFloatInput("Shipping Cost: ", 0, 10000);
                
                getStringInput("Order Priority: ", newRecord.order_priority, STR_LENGTH);
                getStringInput("Payment Method: ", newRecord.payment_method, STR_LENGTH);
                
                if (insertRecord(&directory, newRecord)) {
                    printf("\nRecord added successfully!\n");
                } else {
                    printf("\nFailed to add record (directory full).\n");
                }
                waitForEnter();
                break;
                
            case 2: // Search record
                printf("\n--- Search Record ---\n");
                customer_id = getIntInput("Enter Customer ID: ", 1, 999999);
                HashNode* foundNode = searchRecord(&directory, customer_id);
                if (foundNode != NULL) {
                    printf("\nRecord found:\n");
                    displayRecord(&foundNode->record);
                } else {
                    printf("\nRecord not found.\n");
                }
                waitForEnter();
                break;
                
            case 3: // Delete record
                printf("\n--- Delete Record ---\n");
                customer_id = getIntInput("Enter Customer ID to delete: ", 1, 999999);
                if (deleteRecord(&directory, customer_id)) {
                    printf("\nRecord deleted successfully.\n");
                } else {
                    printf("\nRecord not found.\n");
                }
                waitForEnter();
                break;
                
            case 4: // Modify record
                printf("\n--- Modify Record ---\n");
                customer_id = getIntInput("Enter Customer ID to modify: ", 1, 999999);
                if (modifyRecord(&directory, customer_id)) {
                    printf("\nRecord modified successfully.\n");
                } else {
                    printf("\nRecord not found.\n");
                }
                waitForEnter();
                break;
                
            case 5: // Display all records
                printf("\n--- All Records ---\n");
                displayAllRecords(&directory);
                waitForEnter();
                break;
                
            case 6: // Save and exit
                if (saveToFile(&directory, filename)) {
                    printf("\nData saved successfully. Exiting...\n");
                } else {
                    printf("\nFailed to save data.\n");
                }
                freeHashTable(&directory);
                break;
        }
    } while (choice != 6);
    
    return 0;
}

void initializeDirectory(SalesDirectory *dir) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        dir->table[i] = NULL;
    }
    dir->count = 0;
}

unsigned int hash(int customer_id) {
    return customer_id % TABLE_SIZE;
}

int insertRecord(SalesDirectory *dir, SalesRecord record) {
    if (dir->count >= MAX_RECORDS) {
        return 0;
    }
    
    unsigned int index = hash(record.customer_id);
    HashNode* newNode = (HashNode*)malloc(sizeof(HashNode));
    if (newNode == NULL) {
        return 0;
    }
    
    newNode->record = record;
    newNode->next = dir->table[index];
    dir->table[index] = newNode;
    dir->count++;
    return 1;
}

void displayRecord(const SalesRecord *record) {
    printf("\n+-----------------------------------------------------------+\n");
    printf("| Order Date/Time: %s %s\n", record->order_date, record->time);
    printf("| Customer ID: %d | Gender: %s\n", record->customer_id, record->gender);
    printf("| Device: %s | Login Type: %s\n", record->device_type, record->customer_login_type);
    printf("| Product: %s (%s)\n", record->product, record->product_category);
    printf("| Sales: $%.2f | Qty: %d | Discount: %.2f\n", record->sales, record->quantity, record->discount);
    printf("| Profit: $%.2f | Shipping: $%.2f\n", record->profit, record->shipping_cost);
    printf("| Priority: %s | Payment: %s\n", record->order_priority, record->payment_method);
    printf("+-----------------------------------------------------------+\n");
}

void displayAllRecords(const SalesDirectory *dir) {
    if (dir->count == 0) {
        printf("No records found.\n");
        return;
    }
    
    printf("Total records: %d\n", dir->count);
    for (int i = 0; i < TABLE_SIZE; i++) {
        HashNode* current = dir->table[i];
        while (current != NULL) {
            displayRecord(&current->record);
            current = current->next;
        }
    }
}

HashNode* searchRecord(const SalesDirectory *dir, int customer_id) {
    unsigned int index = hash(customer_id);
    HashNode* current = dir->table[index];
    
    while (current != NULL) {
        if (current->record.customer_id == customer_id) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}

int deleteRecord(SalesDirectory *dir, int customer_id) {
    unsigned int index = hash(customer_id);
    HashNode* current = dir->table[index];
    HashNode* prev = NULL;
    
    while (current != NULL) {
        if (current->record.customer_id == customer_id) {
            if (prev == NULL) {
                dir->table[index] = current->next;
            } else {
                prev->next = current->next;
            }
            free(current);
            dir->count--;
            return 1;
        }
        prev = current;
        current = current->next;
    }
    
    return 0;
}

int modifyRecord(SalesDirectory *dir, int customer_id) {
    HashNode* node = searchRecord(dir, customer_id);
    if (node == NULL) {
        return 0;
    }
    
    SalesRecord *record = &node->record;
    printf("\nCurrent record:\n");
    displayRecord(record);
    
    printf("\nEnter new values (press Enter to keep current):\n");
    
    char input[STR_LENGTH * 2];
    float temp_float;
    int temp_int;
    
    // Date
    do {
        getStringInput("Order Date: ", input, DATE_LENGTH);
        if (strlen(input) > 0 && !validateDate(input)) {
            printf("Invalid date format. ");
            continue;
        }
        if (strlen(input) > 0) strcpy(record->order_date, input);
        break;
    } while (1);
    
    // Time
    do {
        getStringInput("Time: ", input, TIME_LENGTH);
        if (strlen(input) > 0 && !validateTime(input)) {
            printf("Invalid time format. ");
            continue;
        }
        if (strlen(input) > 0) strcpy(record->time, input);
        break;
    } while (1);
    
    // Aging
    temp_float = getFloatInput("Aging: ", 0, 100);
    if (temp_float != -1) record->aging = temp_float;
    
    // Customer ID
    do {
        temp_int = getIntInput("Customer ID: ", 1, 999999);
        if (temp_int != -1) {
            if (temp_int == record->customer_id) break;
            int result = validateCustomerID(dir, temp_int, node);
            if (result == 0) {
                // Need to rehash if ID changes
                SalesRecord temp = *record;
                temp.customer_id = temp_int;
                deleteRecord(dir, record->customer_id);
                insertRecord(dir, temp);
                return 1;
            } else if (result == -2) {
                printf("ID already exists. ");
            }
        } else {
            break;
        }
    } while (1);
    
    // Other fields
    getStringInput("Gender: ", input, STR_LENGTH);
    if (strlen(input) > 0) strcpy(record->gender, input);
    
    getStringInput("Device Type: ", input, STR_LENGTH);
    if (strlen(input) > 0) strcpy(record->device_type, input);
    
    getStringInput("Login Type: ", input, STR_LENGTH);
    if (strlen(input) > 0) strcpy(record->customer_login_type, input);
    
    getStringInput("Product Category: ", input, STR_LENGTH);
    if (strlen(input) > 0) strcpy(record->product_category, input);
    
    getStringInput("Product: ", input, STR_LENGTH);
    if (strlen(input) > 0) strcpy(record->product, input);
    
    temp_float = getFloatInput("Sales: ", 0, 1000000);
    if (temp_float != -1) record->sales = temp_float;
    
    temp_int = getIntInput("Quantity: ", 1, 1000);
    if (temp_int != -1) record->quantity = temp_int;
    
    do {
        temp_float = getFloatInput("Discount: ", 0, 1);
        if (temp_float != -1) {
            record->discount = temp_float;
            break;
        } else {
            break;
        }
    } while (1);
    
    temp_float = getFloatInput("Profit: ", -1000000, 1000000);
    if (temp_float != -1) record->profit = temp_float;
    
    temp_float = getFloatInput("Shipping Cost: ", 0, 10000);
    if (temp_float != -1) record->shipping_cost = temp_float;
    
    getStringInput("Order Priority: ", input, STR_LENGTH);
    if (strlen(input) > 0) strcpy(record->order_priority, input);
    
    getStringInput("Payment Method: ", input, STR_LENGTH);
    if (strlen(input) > 0) strcpy(record->payment_method, input);
    
    return 1;
}

int saveToFile(const SalesDirectory *dir, const char *filename) {
    FILE *file = fopen(filename, "wb");
    if (!file) return 0;
    
    if (fwrite(&dir->count, sizeof(int), 1, file) != 1) {
        fclose(file);
        return 0;
    }
    
    for (int i = 0; i < TABLE_SIZE; i++) {
        HashNode* current = dir->table[i];
        while (current != NULL) {
            if (fwrite(&current->record, sizeof(SalesRecord), 1, file) != 1) {
                fclose(file);
                return 0;
            }
            current = current->next;
        }
    }
    
    fclose(file);
    return 1;
}

int loadFromFile(SalesDirectory *dir, const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) return 0;
    
    int recordCount;
    if (fread(&recordCount, sizeof(int), 1, file) != 1) {
        fclose(file);
        return 0;
    }
    
    for (int i = 0; i < recordCount; i++) {
        SalesRecord record;
        if (fread(&record, sizeof(SalesRecord), 1, file) != 1) {
            fclose(file);
            return 0;
        }
        if (!insertRecord(dir, record)) {
            fclose(file);
            return 0;
        }
    }
    
    fclose(file);
    return 1;
}

void displayMenu() {
    printf("\n+-------------------------------------+\n");
    printf("|    SALES RECORD MANAGEMENT SYSTEM   |\n");
    printf("+-------------------------------------+\n");
    printf("| 1. Add New Record                  |\n");
    printf("| 2. Search Record                   |\n");
    printf("| 3. Delete Record                   |\n");
    printf("| 4. Modify Record                   |\n");
    printf("| 5. Display All Records             |\n");
    printf("| 6. Save and Exit                   |\n");
    printf("+-------------------------------------+\n");
}

void addSampleData(SalesDirectory *dir) {
    SalesRecord sampleRecords[5] = {
        {"2023-01-15", "10:30:00", 5.0, 1001, "Male", "Mobile", "Member", 
         "Electronics", "Smartphone", 599.99, 1, 0.1, 150.0, 5.99, "High", "Credit"},
        {"2023-02-20", "14:45:30", 3.0, 1002, "Female", "Desktop", "Guest", 
         "Clothing", "T-Shirt", 24.99, 2, 0.0, 10.0, 2.99, "Medium", "PayPal"},
        {"2023-03-10", "09:15:22", 7.0, 1003, "Male", "Tablet", "Member", 
         "Books", "Novel", 12.50, 1, 0.15, 3.75, 0.0, "Low", "Credit"},
        {"2023-04-05", "16:20:45", 2.0, 1004, "Female", "Mobile", "Guest", 
         "Home", "Blender", 45.99, 1, 0.2, 9.20, 4.99, "Medium", "Debit"},
        {"2023-05-12", "11:10:33", 1.0, 1005, "Male", "Desktop", "Member", 
         "Electronics", "Headphones", 89.99, 1, 0.05, 22.50, 0.0, "High", "Credit"}
    };
    
    for (int i = 0; i < 5 && dir->count < MAX_RECORDS; i++) {
        insertRecord(dir, sampleRecords[i]);
    }
}

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void waitForEnter() {
    printf("Press Enter to continue...");
    while (getchar() != '\n');
}

int validateDate(const char *date) {
    if (strlen(date) != 10) return 0;
    if (date[4] != '-' || date[7] != '-') return 0;
    
    int year, month, day;
    if (sscanf(date, "%d-%d-%d", &year, &month, &day) != 3) return 0;
    
    if (year < 2000 || year > 2100) return 0;
    if (month < 1 || month > 12) return 0;
    if (day < 1 || day > 31) return 0;
    
    return 1;
}

int validateTime(const char *time) {
    if (strlen(time) != 8) return 0;
    if (time[2] != ':' || time[5] != ':') return 0;
    
    int hour, minute, second;
    if (sscanf(time, "%d:%d:%d", &hour, &minute, &second) != 3) return 0;
    
    if (hour < 0 || hour > 23) return 0;
    if (minute < 0 || minute > 59) return 0;
    if (second < 0 || second > 59) return 0;
    
    return 1;
}

int validateCustomerID(const SalesDirectory *dir, int id, HashNode* current_node) {
    if (id <= 0) return -1;
    
    unsigned int index = hash(id);
    HashNode* current = dir->table[index];
    
    while (current != NULL) {
        if (current != current_node && current->record.customer_id == id) {
            return -2;
        }
        current = current->next;
    }
    
    return 0;
}

void getStringInput(const char *prompt, char *buffer, int buffer_size) {
    printf("%s", prompt);
    fgets(buffer, buffer_size, stdin);
    buffer[strcspn(buffer, "\n")] = '\0';
}

float getFloatInput(const char *prompt, float min, float max) {
    char input[50];
    float value;
    
    printf("%s", prompt);
    fgets(input, sizeof(input), stdin);
    
    if (strlen(input) <= 1) {
        return -1;
    }
    
    while (sscanf(input, "%f", &value) != 1 || value < min || value > max) {
        printf("Invalid input. Enter a value between %.2f and %.2f: ", min, max);
        fgets(input, sizeof(input), stdin);
    }
    
    return value;
}

int getIntInput(const char *prompt, int min, int max) {
    char input[50];
    int value;
    
    printf("%s", prompt);
    fgets(input, sizeof(input), stdin);
    
    if (strlen(input) <= 1) {
        return -1;
    }
    
    while (sscanf(input, "%d", &value) != 1 || value < min || value > max) {
        printf("Invalid input. Enter a value between %d and %d: ", min, max);
        fgets(input, sizeof(input), stdin);
    }
    
    return value;
}

void freeHashTable(SalesDirectory *dir) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        HashNode* current = dir->table[i];
        while (current != NULL) {
            HashNode* temp = current;
            current = current->next;
            free(temp);
        }
        dir->table[i] = NULL;
    }
    dir->count = 0;
}