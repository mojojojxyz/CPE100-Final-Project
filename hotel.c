#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Enable UTF-8 (Windows)
#ifdef _WIN32
    #define ENABLE_UTF8() system("chcp 65001 > nul");
#else
    #define ENABLE_UTF8()
#endif

/*
================================================================================
Hotel Booking Management System
โครงสร้าง:
1) loadBookings()       - โหลดข้อมูลจากไฟล์ตอนเริ่มโปรแกรม
2) addBooking()         - เพิ่มการจองใหม่ (คำนวณราคาจากประเภทและจำนวนคืน)
3) cancelBooking()      - ยกเลิกการจอง (เปลี่ยน isActive = 0)
4) processPayment()     - จัดการการชำระเงินห้องพัก
5) searchAndDisplayBooking() - ค้นหาและแสดงรายละเอียดการจอง
6) report()             - สรุปรายงานยอดต่าง ๆ
7) saveBookings()       - บันทึกข้อมูลลงไฟล์
8) main()                - เมนูหลักและการควบคุมโปรแกรม
================================================================================
*/

/* -------------------------
   Definitions & data
   ------------------------- */

typedef struct {
    int id;
    char firstName[50];
    char lastName[50];
    char roomType[20];
    float price;                // total room price (price per night * nights)
    char checkIn[20];           // "dd/mm/yyyy"
    char checkOut[20];          // "dd/mm/yyyy"
    int isActive;               // 1 = active, 0 = canceled
    int isPaid;                 // 1 = paid, 0 = unpaid (room price)
} Booking;

Booking bookings[100];
int bookingCount = 0;
int nextId = 1;

/* -------------------------
   Utility Functions
   ------------------------- */

float getRoomPrice(const char *roomType) {
    if (strcmp(roomType, "Standard") == 0)
        return 1200.0;
    else if (strcmp(roomType, "Deluxe") == 0)
        return 1700.0;
    else if (strcmp(roomType, "Suite") == 0)
        return 2200.0;
    else
        return -1; // invalid type
}

void showRoomPrices() {
    printf("\n=== Room Prices ===\n");
    printf("Standard : 1200 THB per night\n");
    printf("Deluxe   : 1700 THB per night\n");
    printf("Suite    : 2200 THB per night\n");
}

int calculateNights(const char *checkIn, const char *checkOut) {
    int d1, m1, y1;
    int d2, m2, y2;

    if (sscanf(checkIn, "%d/%d/%d", &d1, &m1, &y1) != 3) return -9999;
    if (sscanf(checkOut, "%d/%d/%d", &d2, &m2, &y2) != 3) return -9999;

    int daysInMonth[12] = {31,28,31,30,31,30,31,31,30,31,30,31};

    int totalDays1 = y1 * 365;
    for (int i = 0; i < m1 - 1; i++) totalDays1 += daysInMonth[i];
    totalDays1 += d1;

    int totalDays2 = y2 * 365;
    for (int i = 0; i < m2 - 1; i++) totalDays2 += daysInMonth[i];
    totalDays2 += d2;

    return totalDays2 - totalDays1;
}

int findBookingIndex(const char *first, const char *last) {
    for (int i = 0; i < bookingCount; i++) {
        if (strcmp(bookings[i].firstName, first) == 0 &&
            strcmp(bookings[i].lastName, last) == 0) {
            return i;
        }
    }
    return -1;
}

int findBookingByID(int bookingId) {
    for (int i = 0; i < bookingCount; i++) {
        if (bookings[i].id == bookingId) {
            return i;
        }
    }
    return -1;
}

void listBookingsByName(const char *first, const char *last) {
    int found = 0;
    printf("\n=== Bookings found ===\n");
    for (int i = 0; i < bookingCount; i++) {
        if (strcmp(bookings[i].firstName, first) == 0 &&
            strcmp(bookings[i].lastName, last) == 0) {
            printf("ID: %d | %s %s | %s | %s to %s | %s\n",
                   bookings[i].id,
                   bookings[i].firstName,
                   bookings[i].lastName,
                   bookings[i].roomType,
                   bookings[i].checkIn,
                   bookings[i].checkOut,
                   bookings[i].isActive ? "Active" : "Canceled");
            found++;
        }
    }
    if (!found) {
        printf("No bookings found.\n");
    }
}

/* -------------------------
   1) loadBookings()
   ------------------------- */

void loadBookings() {
    FILE *file = fopen("bookings.txt", "r");
    if (!file) {
        printf("No previous booking file found. A new file will be created when saving.\n");
        bookingCount = 0;
        nextId = 1;
        return;
    }

    bookingCount = 0;
    nextId = 1;

    Booking b;

    while (fscanf(file, "%d %49s %49s %19s %f %19s %19s %d %d",
                  &b.id, b.firstName, b.lastName, b.roomType,
                  &b.price, b.checkIn, b.checkOut, &b.isActive,
                  &b.isPaid) == 9) 
    {
        bookings[bookingCount++] = b;

        if (b.id >= nextId)
            nextId = b.id + 1;

        if (bookingCount >= 100)
            break;
    }

    fclose(file);
    printf("Loaded %d booking(s).\n", bookingCount);
}

/* -------------------------
   2) addBooking()
   ------------------------- */

void addBooking() {
    Booking b;
    b.id = nextId++;

    printf("\n=== Add New Booking ===\n");
    printf("First Name: ");
    scanf("%49s", b.firstName);

    printf("Last Name: ");
    scanf("%49s", b.lastName);

    showRoomPrices();
    printf("\nRoom Type (Standard/Deluxe/Suite): ");
    scanf("%19s", b.roomType);

    float roomPrice = getRoomPrice(b.roomType);
    if (roomPrice < 0) {
        printf("Invalid room type!\n");
        return;
    }

    printf("Check-in Date (dd/mm/yyyy): ");
    scanf("%19s", b.checkIn);

    printf("Check-out Date (dd/mm/yyyy): ");
    scanf("%19s", b.checkOut);

    int nights = calculateNights(b.checkIn, b.checkOut);
    if (nights <= 0) {
        printf("Error: Check-out must be after Check-in!\n");
        return;
    }

    b.price = roomPrice * nights;

    printf("\nNights Stayed: %d night(s)\n", nights);
    printf("Room Price per Night: %.2f THB\n", roomPrice);
    printf("Total Room Price: %.2f THB\n", b.price);

    b.isActive = 1;
    b.isPaid = 0;

    if (bookingCount < 100) {
        bookings[bookingCount++] = b;
        printf("\n✓ Booking added successfully!\n");
        printf("Booking ID: %d\n", b.id);
        printf("Payment Status: UNPAID\n");
        printf("Use menu option 3 to process payment.\n");
    } else {
        printf("Error: Booking storage full.\n");
    }
}

/* -------------------------
   3) cancelBooking()
   ------------------------- */

void cancelBooking() {
    printf("\n=== Cancel Booking ===\n");
    printf("Search by:\n");
    printf("1. Name\n");
    printf("2. Booking ID\n");
    printf("Select option: ");
    
    int searchOption;
    if (scanf("%d", &searchOption) != 1) {
        int c;
        while ((c = getchar()) != '\n' && c != EOF) {}
        printf("Invalid input.\n");
        return;
    }

    int idx = -1;
    
    if (searchOption == 1) {
        char first[50], last[50];
        printf("First Name: ");
        scanf("%49s", first);
        printf("Last Name: ");
        scanf("%49s", last);
        
        idx = findBookingIndex(first, last);
        
        int count = 0;
        for (int i = 0; i < bookingCount; i++) {
            if (strcmp(bookings[i].firstName, first) == 0 &&
                strcmp(bookings[i].lastName, last) == 0) {
                count++;
            }
        }
        
        if (count > 1) {
            printf("\nFound %d bookings with this name!\n", count);
            listBookingsByName(first, last);
            printf("\nPlease enter Booking ID: ");
            int bookingId;
            scanf("%d", &bookingId);
            idx = findBookingByID(bookingId);
        }
    } else if (searchOption == 2) {
        int bookingId;
        printf("Booking ID: ");
        scanf("%d", &bookingId);
        idx = findBookingByID(bookingId);
    } else {
        printf("Invalid option.\n");
        return;
    }

    if (idx == -1) {
        printf("Booking not found.\n");
        return;
    }

    if (!bookings[idx].isActive) {
        printf("This booking has already been canceled.\n");
        return;
    }

    Booking *b = &bookings[idx];
    printf("\nBooking to cancel:\n");
    printf("  ID: %d\n", b->id);
    printf("  Name: %s %s\n", b->firstName, b->lastName);
    printf("  Room: %s\n", b->roomType);
    printf("  Total Amount: %.2f THB\n", b->price);

    char confirm;
    printf("\nConfirm cancellation? (y/n): ");
    scanf(" %c", &confirm);

    if (confirm == 'y' || confirm == 'Y') {
        bookings[idx].isActive = 0;
        printf("✓ Booking canceled successfully.\n");
    } else {
        printf("Cancellation aborted.\n");
    }
}

/* -------------------------
   4) processPayment()
   ------------------------- */

void processPayment() {
    printf("\n=== Process Payment ===\n");
    printf("Search by:\n");
    printf("1. Name\n");
    printf("2. Booking ID\n");
    printf("Select option: ");
    
    int searchOption;
    if (scanf("%d", &searchOption) != 1) {
        int c;
        while ((c = getchar()) != '\n' && c != EOF) {}
        printf("Invalid input.\n");
        return;
    }

    int idx = -1;
    
    if (searchOption == 1) {
        char first[50], last[50];
        printf("First Name: ");
        scanf("%49s", first);
        printf("Last Name: ");
        scanf("%49s", last);
        
        idx = findBookingIndex(first, last);
        
        int count = 0;
        for (int i = 0; i < bookingCount; i++) {
            if (strcmp(bookings[i].firstName, first) == 0 &&
                strcmp(bookings[i].lastName, last) == 0) {
                count++;
            }
        }
        
        if (count > 1) {
            printf("\nFound %d bookings with this name!\n", count);
            listBookingsByName(first, last);
            printf("\nPlease enter Booking ID: ");
            int bookingId;
            scanf("%d", &bookingId);
            idx = findBookingByID(bookingId);
        }
    } else if (searchOption == 2) {
        int bookingId;
        printf("Booking ID: ");
        scanf("%d", &bookingId);
        idx = findBookingByID(bookingId);
    } else {
        printf("Invalid option.\n");
        return;
    }

    if (idx == -1) {
        printf("Booking not found.\n");
        return;
    }

    if (!bookings[idx].isActive) {
        printf("Error: Cannot process payment for a canceled booking.\n");
        return;
    }

    Booking *b = &bookings[idx];

    printf("\n----- Booking Information -----\n");
    printf("Booking ID: %d\n", b->id);
    printf("Name: %s %s\n", b->firstName, b->lastName);
    printf("Room Type: %s\n", b->roomType);
    printf("Room Price: %.2f THB\n", b->price);
    if (b->isPaid) {
        printf("Current Payment Status : PAID\n");
    } else {
        printf("Current Payment Status : UNPAID\n");
    }
    printf("-------------------------------\n");

    if (b->isPaid) {
        printf("\n ✓ Room price has already been paid.\n");
        return;
    }

    char confirm;
    printf("\nConfirm payment of %.2f THB? (y/n): ", b->price);
    scanf(" %c", &confirm);
    
    if (confirm == 'y' || confirm == 'Y') {
        b->isPaid = 1;
        printf(" ✓ Room payment processed successfully!\n");
        printf(" ✓ All payments completed!\n");
    } else {
        printf("Payment canceled.\n");
    }
}

/* -------------------------
   5) searchAndDisplayBooking()
   ------------------------- */

void searchAndDisplayBooking() {
    printf("\n=== Search Booking ===\n");
    printf("Search by:\n");
    printf("1. Name\n");
    printf("2. Booking ID\n");
    printf("Select option: ");
    
    int searchOption;
    if (scanf("%d", &searchOption) != 1) {
        int c;
        while ((c = getchar()) != '\n' && c != EOF) {}
        printf("Invalid input.\n");
        return;
    }

    int idx = -1;
    
    if (searchOption == 1) {
        char first[50], last[50];
        printf("First Name: ");
        scanf("%49s", first);
        printf("Last Name: ");
        scanf("%49s", last);
        
        idx = findBookingIndex(first, last);
        
        int count = 0;
        for (int i = 0; i < bookingCount; i++) {
            if (strcmp(bookings[i].firstName, first) == 0 &&
                strcmp(bookings[i].lastName, last) == 0) {
                count++;
            }
        }
        
        if (count > 1) {
            printf("\nFound %d bookings with this name!\n", count);
            listBookingsByName(first, last);
            printf("\nPlease enter Booking ID: ");
            int bookingId;
            scanf("%d", &bookingId);
            idx = findBookingByID(bookingId);
        }
    } else if (searchOption == 2) {
        int bookingId;
        printf("Booking ID: ");
        scanf("%d", &bookingId);
        idx = findBookingByID(bookingId);
    } else {
        printf("Invalid option.\n");
        return;
    }

    if (idx == -1) {
        printf("Booking not found.\n");
        return;
    }

    Booking *b = &bookings[idx];
    printf("\n===== Booking Details =====\n");
    printf("Booking ID        : %d\n", b->id);
    printf("Name              : %s %s\n", b->firstName, b->lastName);
    printf("Room Type         : %s\n", b->roomType);
    printf("Check-in          : %s\n", b->checkIn);
    printf("Check-out         : %s\n", b->checkOut);
    
    if (b->isActive) {
        printf("Status            : Active\n");
    } else {
        printf("Status            : Canceled\n");
    }

    if (b->isActive) {
        char paymentStatus[10];
        if (b->isPaid) {
            strcpy(paymentStatus, "PAID");
        } else {
            strcpy(paymentStatus, "UNPAID");
        }
        printf("Room Price        : %.2f THB [%s]\n", b->price, paymentStatus);
        
        if (b->isPaid) {
            printf("\n ✓ Payment completed!\n");
        } else {
            printf("\n Outstanding: %.2f THB (Room not paid)\n", b->price);
        }
    } else {
        printf("Room Price        : %.2f THB (Canceled)\n", b->price);
    }
    printf("===========================\n");
}

/* -------------------------
   6) report()
   ------------------------- */

void report() {
    printf("\n=== Summary Report ===\n");

    int active = 0;
    int canceled = 0;
    float paidRevenue = 0.0f;
    float unpaidRevenue = 0.0f;
    float lostRevenue = 0.0f;

    for (int i = 0; i < bookingCount; i++) {
        if (bookings[i].isActive) {
            active++;
            if (bookings[i].isPaid) {
                paidRevenue += bookings[i].price;
            } else {
                unpaidRevenue += bookings[i].price;
            }
        } else {
            canceled++;
            lostRevenue += bookings[i].price;
        }
    }

    printf("Total Bookings      : %d\n", bookingCount);
    printf("Active Bookings     : %d\n", active);
    printf("Canceled Bookings   : %d\n", canceled);
    printf("\n--- Revenue Breakdown ---\n");
    printf("Paid Revenue        : %.2f THB\n", paidRevenue);
    printf("Unpaid Revenue      : %.2f THB\n", unpaidRevenue);
    printf("Total Active Revenue: %.2f THB\n", paidRevenue + unpaidRevenue);
    printf("Lost Revenue        : %.2f THB (from cancellations)\n", lostRevenue);
}

/* -------------------------
   7) saveBookings()
   ------------------------- */

void saveBookings() {
    FILE *file = fopen("bookings.txt", "w");
    if (!file) {
        printf("Error: Could not save booking data!\n");
        return;
    }

    for (int i = 0; i < bookingCount; i++) {
        Booking *b = &bookings[i];
        fprintf(file, "%d %s %s %s %.2f %s %s %d %d\n",
                b->id, b->firstName, b->lastName,
                b->roomType, b->price, b->checkIn,
                b->checkOut, b->isActive, b->isPaid);
    }

    fclose(file);
    printf("Booking data saved.\n");
}

/* -------------------------
   main() - Menu & Control Flow
   ------------------------- */

int main() {
    ENABLE_UTF8();
    loadBookings();

    int choice;
    while (1) {
        printf("\n==============================\n");
        printf("   Hotel Booking Management   \n");
        printf("==============================\n");
        printf("1. Add Booking\n");
        printf("2. Cancel Booking\n");
        printf("3. Process Payment\n");
        printf("4. Search and Display Booking\n");
        printf("5. View Summary Report\n");
        printf("6. Save Bookings\n");
        printf("7. Exit Program\n");
        printf("Select option: ");
        if (scanf("%d", &choice) != 1) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF) {}
            printf("Invalid input. Please enter a number.\n");
            continue;
        }

        switch (choice) {
            case 1: addBooking(); break;
            case 2: cancelBooking(); break;
            case 3: processPayment(); break;
            case 4: searchAndDisplayBooking(); break;
            case 5: report(); break;
            case 6: saveBookings(); break;
            case 7:
                printf("Exiting...\n");
                return 0;
            default:
                printf("Invalid option.\n");
        }
    }
    return 0;
}