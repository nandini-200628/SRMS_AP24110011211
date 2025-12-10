#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32) || defined(_WIN64)
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

#define STUD_FILE "students.txt"

struct Student {
    int roll;
    char name[50];
    float marks;
};

char currentRole[20];
int currentRoll;
char currentName[50];

/* -------------------------------------------------------------------
   PASSWORD INPUT WITH * MASKING  (Windows + Linux supported)
------------------------------------------------------------------- */
void getPassword(char *pass, int size) {
    int idx = 0;
    int ch;

#if defined(_WIN32) || defined(_WIN64)

    while (1) {
        ch = _getch();
        if (ch == 13) break;    // Enter key
        if (ch == 8) {          // Backspace
            if (idx > 0) {
                idx--;
                printf("\b \b");
            }
        } else if (idx < size - 1) {
            pass[idx++] = ch;
            printf("*");
        }
    }

#else
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    while ((ch = getchar()) != '\n') {
        if (ch == 127 || ch == 8) {
            if (idx > 0) {
                idx--;
                printf("\b \b");
            }
        } else if (idx < size - 1) {
            pass[idx++] = ch;
            printf("*");
        }
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
#endif

    pass[idx] = '\0';
    printf("\n");
}

/* -------------------------------------------------------------------
     FILE OPERATIONS
------------------------------------------------------------------- */

void addStudent() {
    FILE *fp = fopen(STUD_FILE, "a");
    if (!fp) {
        printf("Error opening file!\n");
        return;
    }

    struct Student s;
    printf("Enter Roll No: ");
    scanf("%d", &s.roll);
    printf("Enter Name: ");
    scanf("%s", s.name);
    printf("Enter Marks: ");
    scanf("%f", &s.marks);

    fprintf(fp, "%d %s %.2f\n", s.roll, s.name, s.marks);
    fclose(fp);

    printf("Student added successfully!\n");
}

void displayStudents() {
    FILE *fp = fopen(STUD_FILE, "r");
    if (!fp) {
        printf("No data found!\n");
        return;
    }

    struct Student s;
    printf("\nROLL\tNAME\tMARKS\n");
    printf("---------------------------\n");

    while (fscanf(fp, "%d %s %f", &s.roll, s.name, &s.marks) == 3) {
        printf("%d\t%s\t%.2f\n", s.roll, s.name, s.marks);
    }

    fclose(fp);
}

void searchStudent() {
    int r;
    printf("Enter roll number to search: ");
    scanf("%d", &r);

    FILE *fp = fopen(STUD_FILE, "r");
    if (!fp) {
        printf("No data found!\n");
        return;
    }

    struct Student s;
    int found = 0;

    while (fscanf(fp, "%d %s %f", &s.roll, s.name, &s.marks) == 3) {
        if (s.roll == r) {
            printf("FOUND: %d %s %.2f\n", s.roll, s.name, s.marks);
            found = 1;
            break;
        }
    }

    if (!found) printf("Student not found!\n");

    fclose(fp);
}

void updateStudent() {
    int r;
    printf("Enter roll number to update: ");
    scanf("%d", &r);

    FILE *fp = fopen(STUD_FILE, "r");
    FILE *temp = fopen("temp.txt", "w");
    if (!fp || !temp) {
        printf("File error!\n");
        return;
    }

    struct Student s;
    int found = 0;

    while (fscanf(fp, "%d %s %f", &s.roll, s.name, &s.marks) == 3) {
        if (s.roll == r) {
            found = 1;
            printf("Enter new Name: ");
            scanf("%s", s.name);
            printf("Enter new Marks: ");
            scanf("%f", &s.marks);
        }
        fprintf(temp, "%d %s %.2f\n", s.roll, s.name, s.marks);
    }

    fclose(fp);
    fclose(temp);

    remove(STUD_FILE);
    rename("temp.txt", STUD_FILE);

    if (found)
        printf("Student updated!\n");
    else
        printf("Student not found!\n");
}

void deleteStudent() {
    int r;
    printf("Enter roll number to delete: ");
    scanf("%d", &r);

    FILE *fp = fopen(STUD_FILE, "r");
    FILE *temp = fopen("temp.txt", "w");
    if (!fp || !temp) {
        printf("File error!\n");
        return;
    }

    struct Student s;
    int found = 0;

    while (fscanf(fp, "%d %s %f", &s.roll, s.name, &s.marks) == 3) {
        if (s.roll == r)
            found = 1;
        else
            fprintf(temp, "%d %s %.2f\n", s.roll, s.name, s.marks);
    }

    fclose(fp);
    fclose(temp);

    remove(STUD_FILE);
    rename("temp.txt", STUD_FILE);

    if (found)
        printf("Deleted successfully!\n");
    else
        printf("Student not found!\n");
}

/* -------------------------------------------------------------------
     LOGIN SYSTEM
------------------------------------------------------------------- */

int login() {
    char username[50], password[50];

    printf("===== LOGIN =====\n");
    printf("Enter Username/Roll: ");
    scanf("%s", username);

    printf("Enter Password: ");
    getPassword(password, 50);

    /* ---- Admin Login ---- */
    if (strcmp(username, "admin") == 0 && strcmp(password, "admin123") == 0) {
        strcpy(currentRole, "admin");
        return 1;
    }

    /* ---- Teacher Login ---- */
    if (strcmp(username, "teacher") == 0 && strcmp(password, "teacher123") == 0) {
        strcpy(currentRole, "teacher");
        return 1;
    }

    /* ---- Student Login (username = roll, password = roll) ---- */
    FILE *fp = fopen(STUD_FILE, "r");
    if (!fp) return 0;

    struct Student s;
    while (fscanf(fp, "%d %s %f", &s.roll, s.name, &s.marks) == 3) {
        char rollStr[10];
        sprintf(rollStr, "%d", s.roll);

        if (strcmp(username, rollStr) == 0 && strcmp(password, rollStr) == 0) {
            strcpy(currentRole, "student");
            currentRoll = s.roll;
            strcpy(currentName, s.name);
            fclose(fp);
            return 1;
        }
    }

    fclose(fp);
    return 0;
}

/* -------------------------------------------------------------------
     MENUS
------------------------------------------------------------------- */

void adminMenu() {
    int ch;
    while (1) {
        printf("\n===== ADMIN MENU =====\n");
        printf("1. Add Student\n");
        printf("2. Display Students\n");
        printf("3. Search Student\n");
        printf("4. Update Student\n");
        printf("5. Delete Student\n");
        printf("6. Logout\n");
        printf("Enter choice: ");
        scanf("%d", &ch);

        if (ch == 1) addStudent();
        else if (ch == 2) displayStudents();
        else if (ch == 3) searchStudent();
        else if (ch == 4) updateStudent();
        else if (ch == 5) deleteStudent();
        else if (ch == 6) break;
        else printf("Invalid choice!\n");
    }
}

void teacherMenu() {
    int ch;
    while (1) {
        printf("\n===== TEACHER MENU =====\n");
        printf("1. Display All Students\n");
        printf("2. Logout\n");
        printf("Enter choice: ");
        scanf("%d", &ch);

        if (ch == 1) displayStudents();
        else if (ch == 2) break;
        else printf("Invalid choice!\n");
    }
}

void studentMenu() {
    FILE *fp = fopen(STUD_FILE, "r");
    if (!fp) {
        printf("No data found!\n");
        return;
    }

    struct Student s;

    while (fscanf(fp, "%d %s %f", &s.roll, s.name, &s.marks) == 3) {
        if (s.roll == currentRoll) {
            printf("\n===== YOUR DETAILS =====\n");
            printf("Roll: %d\n", s.roll);
            printf("Name: %s\n", s.name);
            printf("Marks: %.2f\n", s.marks);
        }
    }

    fclose(fp);
}

/* -------------------------------------------------------------------
     MAIN
------------------------------------------------------------------- */

int main() {
    if (!login()) {
        printf("\nLogin Failed. Exiting...\n");
        return 0;
    }

    if (strcmp(currentRole, "admin") == 0)
        adminMenu();
    else if (strcmp(currentRole, "teacher") == 0)
        teacherMenu();
    else
        studentMenu();

    return 0;
}
