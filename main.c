#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <conio.h>
#include <time.h>

#define MAX_USERS 100
#define MAX_SESSIONS 100
#define MAX_ATTENDANCE 1000
#define MAX_PRN_LEN 9
#define MAX_USERNAME_LEN 30
#define MAX_SUBJECT_LEN 8
#define SESSION_LIFETIME 20
#define MAX_PASSWORD_LEN 30
#define MAX_SESSION_CODE_LEN 20
#define FILENAME_USERS "users.dat"
#define FILENAME_SESSIONS "sessions.dat"
#define FILENAME_ATTENDANCE "attendance.dat"
#define FILENAME_LOG "log.txt"

typedef union
{
    char PRN[MAX_PRN_LEN];
    char subject[MAX_SUBJECT_LEN]; //shortforms only
} Details;

typedef struct {
    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];
    char role[8]; // teacher or student
    Details id;
} User;

typedef struct {
    char sessionCode[MAX_SESSION_CODE_LEN];  // Unique session code
    char subject[MAX_SUBJECT_LEN];  // Name of the session 
    char date[11];         // Date of the session 
    char time[6];         // Time of the session
    char createdBy[30];    // Teacher username
} AttendanceSession;

typedef struct {  
    char username[MAX_USERNAME_LEN];
    char subject[MAX_SUBJECT_LEN];  
    char sessionCode[MAX_SESSION_CODE_LEN];      // Session code
    char PRN[MAX_PRN_LEN];  // Student prn
    char time[6];
    char date[11];
    char status[8];           // Present or Absent
} Attendance;

User users[MAX_USERS];
AttendanceSession sessions[MAX_SESSIONS];
Attendance attendanceRecords[MAX_ATTENDANCE];

int userCount = 0, sessionCount = 0, attendanceCount = 0;


// Function implementations
void logEvent(char *role,char *name,char *event) {
    FILE *file = fopen(FILENAME_LOG, "a"); // Open the file in append mode
    if (file == NULL) {
        printf("Error opening file!\n");
        return;
    }

    char dateStr[11]; 
    char timeStr[9];   
    time_t now;
    time(&now);
    struct tm *local = localtime(&now);
    sprintf(dateStr, "%04d-%02d-%02d", local->tm_year + 1900, local->tm_mon + 1, local->tm_mday);
    sprintf(timeStr, "%02d:%02d", local->tm_hour, local->tm_min); 
    
    fprintf(file, "[%s] %s %s at: %s %s\n",role,name,event,dateStr,timeStr);

    fclose(file); 
}

void saveUsersToFile() {
    FILE *file = fopen(FILENAME_USERS, "wb");
    if (file == NULL) {
        printf("Error opening file for writing.\n");
        return;
    }
    fwrite(users, sizeof(User), userCount, file);
    fclose(file);
}

void loadUsersFromFile() {
    FILE *file = fopen(FILENAME_USERS, "rb");
    if (file == NULL) {
        return;
    }
    userCount = fread(users, sizeof(User), MAX_USERS, file);
    fclose(file);
}

void saveSessionsToFile() {
    FILE *file = fopen(FILENAME_SESSIONS, "wb");
    if (file == NULL) {
        printf("Error opening file for saving sessions.\n");
        return;
    }
    fwrite(sessions, sizeof(AttendanceSession), sessionCount, file);
    fclose(file);
}

void loadSessionsFromFile() {
    FILE *file = fopen(FILENAME_SESSIONS, "rb");
    if (file == NULL) {
        return;
    }
    sessionCount = fread(sessions, sizeof(AttendanceSession), MAX_SESSIONS, file);
    fclose(file);
}

void saveAttendanceToFile() {
    FILE *file = fopen(FILENAME_ATTENDANCE, "wb");
    if (file == NULL) {
        printf("Error opening file for saving attendance.\n");
        return;
    }
    fwrite(attendanceRecords, sizeof(Attendance), attendanceCount, file);
    fclose(file);
}

void loadAttendanceFromFile() {
    FILE *file = fopen(FILENAME_ATTENDANCE, "rb");
    if (file == NULL) {
        return;
    }
    attendanceCount = fread(attendanceRecords, sizeof(Attendance), MAX_ATTENDANCE, file);
    fclose(file);
}

void clearScreen() {
    system("cls");
}

void getPassword(char *password) {
    int i = 0;
    char ch;
    while ((ch = _getch()) != '\r') { 
        if (ch == '\b') { 
            if (i > 0) {
                i--;
                printf("\b \b");
            }
        } else if (i < MAX_PASSWORD_LEN - 1) {
            password[i++] = ch;
            printf("*");
        }
    }
    password[i] = '\0';
    printf("\n");
}


void removeElement(AttendanceSession sessions[], int *size, int index) {
    if (index < 0 || index >= *size) {
        printf("Index out of bounds.\n");
        return;
    }
    
    // Shift all elements after the removed element to the left
    for (int i = index; i < *size - 1; i++) {
        sessions[i] = sessions[i + 1];  // Shift the next element to the current index
    }

    // Optionally zero out the last element (to avoid leftover data)
    memset(&sessions[*size - 1], 0, sizeof(AttendanceSession));
    
    (*size)--;  // Decrease the size of the array
}

void removeExpiredSessions(){
    char dateStr[11]; 
    char timeStr[9];   
    time_t now;
    time(&now);
    struct tm *local = localtime(&now);
    sprintf(dateStr, "%04d-%02d-%02d", local->tm_year + 1900, local->tm_mon + 1, local->tm_mday);
    sprintf(timeStr, "%02d:%02d", local->tm_hour, local->tm_min); 
    char *Hr = strtok(timeStr, ":");
    char *Min = strtok(NULL, ":");
    int hours = atoi(Hr);
    int minutes = atoi(Min);
    int totalMinutes = (hours * 60) + minutes;
    AttendanceSession expiredSessions[100];
    int expiredSessionCount = 0;
    for (int i = 0; i < sessionCount; i++) {
        if(strcmp(sessions[i].date,dateStr) != 0){
            expiredSessions[expiredSessionCount] = sessions[i];
            expiredSessionCount++;

            removeElement(sessions,&sessionCount,i);
            i--;

            continue;
        }
        char sessionTime[5];
        strcpy(sessionTime,sessions[i].time);
        char *h = strtok(sessionTime, ":");
        char *m = strtok(NULL, ":");
        int hour = atoi(h);
        int min = atoi(m);
        int totalSessionMin = (hour * 60) + min;

        if(totalMinutes - totalSessionMin >= SESSION_LIFETIME){
            expiredSessions[expiredSessionCount] = sessions[i];
            expiredSessionCount++;
            removeElement(sessions,&sessionCount,i);
            i--;
            continue;
        }
        
    }
    saveSessionsToFile();
}

void createAttendanceSession(int i) {
    removeExpiredSessions();
    if (sessionCount >= MAX_SESSIONS) {
        printf("Session limit reached. Cannot create a new session.\n");
        return;
    }

    AttendanceSession newSession;

    time_t now;
    time(&now);
    struct tm *local = localtime(&now);
    char dateStr[11]; 
    char timeStr[9]; 
    sprintf(dateStr, "%04d-%02d-%02d", local->tm_year + 1900, local->tm_mon + 1, local->tm_mday);
    sprintf(timeStr, "%02d:%02d", local->tm_hour, local->tm_min);

    
    printf("Enter Session Code: ");
    scanf(" %[^\n]s", newSession.sessionCode);
    strcpy(newSession.subject,users[i].id.subject);
    for (int i = 0; i < sessionCount; i++) {
        if (strcmp(sessions[i].sessionCode, newSession.sessionCode) == 0) {
            printf("Session Code already exists. Try again.\n");
            return;
        }
    }
    
    strcpy(newSession.date,dateStr);
    strcpy(newSession.time,timeStr);
    strcpy(newSession.createdBy, users[i].username);
    sessions[sessionCount++] = newSession;
    saveSessionsToFile();

    for(int i = 0;i<userCount;i++){
        if(strcmp(users[i].role,"student") == 0){
            Attendance newAttendance;
            strcpy(newAttendance.username,users[i].username);
            strcpy(newAttendance.sessionCode, newSession.sessionCode);
            strcpy(newAttendance.subject, newSession.subject);
            strcpy(newAttendance.PRN, users[i].id.PRN);
            strcpy(newAttendance.date, newSession.date);
            strcpy(newAttendance.time, newSession.time);
            strcpy(newAttendance.status, "Absent");
            attendanceRecords[attendanceCount++] = newAttendance;
            saveAttendanceToFile();
        }
    }
    printf("Attendance session created successfully! on %s %s\n",timeStr,dateStr);
}


void viewSessionsForAttendance(char *PRN) {
    //Remove Expired Sessions from file
    removeExpiredSessions();

    if (sessionCount == 0) {
        printf("No sessions available.\n");
        return;
    }

    printf("Available Sessions:\n");
    for (int i = 0; i < sessionCount; i++) {
        printf("%d. %s - %s at %s\n", i + 1, sessions[i].subject, sessions[i].date, sessions[i].time);
    }

    char sessionCode[20];
    printf("Enter Session Code to mark attendance: ");
    scanf("%s", sessionCode);

    for (int i = 0; i < sessionCount; i++) {
        if (strcmp(sessions[i].sessionCode, sessionCode) == 0) {
            for(int j =0;j<attendanceCount;j++){
                if(strcmp(sessionCode,attendanceRecords[j].sessionCode) == 0 && strcmp(sessions[i].subject,attendanceRecords[j].subject) == 0 && strcmp(PRN,attendanceRecords[j].PRN) == 0 && strcmp(sessions[i].date,attendanceRecords[j].date) == 0 && strcmp(sessions[i].time,attendanceRecords[j].time) == 0){

                    strcpy(attendanceRecords[j].status,"Present");
                    saveAttendanceToFile();
                    return;
                }
            }
            
            printf("Attendance marked successfully for session: %s\n", sessionCode);
            return;
        }
    }

    printf("\nInvalid Session Code.\n");
}

void getAttendanceOfSubject(char *subject,char *PRN){
    printf("\nAttendance in %s\n",subject);
    int present = 0;
    int absent = 0;
    for (int i = 0; i < attendanceCount; i++) {
        if (strcmp(attendanceRecords[i].PRN, PRN) == 0 && strcmp(attendanceRecords[i].subject, subject) == 0) {
            if(strcmp(attendanceRecords[i].status,"Present") == 0){
                present++;
            }else{
                absent++;
            }
            printf("%s %s : %s \n",attendanceRecords[i].date,attendanceRecords[i].time,attendanceRecords[i].status);
        }
    }
    int total = absent + present;
    if(total == 0){return;}
    printf("Percent of Sessions Attended: %.2f\n\n",((float)present/total) * 100);
}

void viewStudentAttendanceHistory(char *PRN) {
    printf("\nAttendance History:\n");
    char subjects[10][8];
    int subCount = 0;

    for (int i = 0; i < attendanceCount; i++) {
        if (strcmp(attendanceRecords[i].PRN, PRN) == 0) {
            int found = 0;
            for(int j=0;j<subCount;j++){
                if(strcmp(attendanceRecords[i].subject,subjects[j]) == 0){
                    found = 1;
                    break;
                }   
            }
            if(!found){
                strcpy(subjects[subCount++],attendanceRecords[i].subject);  
            }

        }
    }

    int present = 0;
    int absent = 0;
    for (int i = 0; i < attendanceCount; i++) {
        if (strcmp(attendanceRecords[i].PRN, PRN) == 0) {
            if(strcmp(attendanceRecords[i].status,"Present") == 0){
                present++;
            }else{
                absent++;
            }
        }
    }
    float percent;
    int total = absent + present;
    if(total == 0){percent = 0;}
    else{percent = ((float)present/total) * 100;}
    

    int choice;
    int run = 1;
    while(run){
        for(int i =0;i<subCount;i++){
            printf("%d. %s \n",i+1,subjects[i]);
        }
        printf("0. Logout\n");
        printf("Overall Attendance: %.2f\n",percent);
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 0:
                run = 0;
                break;
            case 1 ... 10:
                if(choice <= subCount){
                getAttendanceOfSubject(subjects[choice-1],PRN);
                break;}
            default:
                printf("Invalid choice.\n");
                break;
        }

    }
}

void getSessionAttendance(AttendanceSession sessionInfo){
    printf("\n");
    int present = 0;
    int absent = 0;
    for (int i = 0; i < attendanceCount; i++) {
        if (strcmp(attendanceRecords[i].subject, sessionInfo.subject) == 0 && strcmp(sessionInfo.date,attendanceRecords[i].date) == 0 &&  strcmp(sessionInfo.time,attendanceRecords[i].time) == 0 && strcmp(sessionInfo.time,attendanceRecords[i].time) == 0 && strcmp(sessionInfo.sessionCode,attendanceRecords[i].sessionCode) == 0) {
            if(strcmp(attendanceRecords[i].status,"Present") == 0){
                present++;
            }else{
                absent++;
            }
            printf("%s %s : %s\n",attendanceRecords[i].username,attendanceRecords[i].PRN,attendanceRecords[i].status);
        }
    }

    int total = absent + present;
    if(total == 0){return;}
    printf("Percent of Students Present: %.2f\n\n",((float)present/total) * 100);
    printf("\n");
}

void checkAttendanceBySession(char *subject){

    AttendanceSession sessionList[100];
    int sessionListCount = 0;
    for (int i = 0; i < attendanceCount; i++) {
        if (strcmp(attendanceRecords[i].subject, subject) == 0) {
            AttendanceSession new;
            strcpy(new.sessionCode,attendanceRecords[i].sessionCode);
            strcpy(new.subject,subject);
            strcpy(new.date,attendanceRecords[i].date);
            strcpy(new.time,attendanceRecords[i].time);
            sessionList[sessionListCount++] = new;
        }
    }

    int choice;
    int run = 1;
    while(run){
        for(int i =0;i<sessionListCount;i++){
            printf("%d. %s %s\n",i+1,sessionList[i].date,sessionList[i].time);
        }
        printf("0. Logout\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 0:
                run = 0;
                break;
            case 1 ... 100:
                if(choice <= sessionListCount){
                    getSessionAttendance(sessionList[choice-1]);
                }
                break;
            default:
                printf("Invalid choice.\n");
                break;
        }

    }


}

void checkAttendanceByStudent(char *subject){
    char PRN[9]; 
    printf("\nEnter 8-digit PRN of Student: ");
    scanf("%s",PRN);
    viewStudentAttendanceHistory(PRN);
    
}


void teacherMenu(int i) {
    logEvent("teacher",users[i].username,"logged in");
    int choice;
    int run = 1;
    while (run) {
        printf("\nTeacher Menu:\n");
        printf("1. Create Attendance Session\n");
        printf("2. Check Attendance by Session\n");
        printf("3. Check Attendance by Student\n");
        printf("4. Logout\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                createAttendanceSession(i);
                break;
            case 2:
                checkAttendanceBySession(users[i].id.subject);
                break;
            case 3:
                checkAttendanceByStudent(users[i].id.subject);
                break;
            case 4:
                logEvent("teacher",users[i].username,"logged out");
                run = 0;
                break;
            default:
                printf("Invalid choice.\n");
                break;
        }

    }
}

void studentMenu(int i) {
    char logTitle[MAX_USERNAME_LEN+MAX_PRN_LEN];
    sprintf(logTitle,"%s %s",users[i].username,users[i].id.PRN);
    logEvent("student",logTitle,"logged in");
    int choice;
    int run = 1;
    while (run) {
        printf("\nStudent Menu:\n");
        printf("1. Mark Attendance\n");
        printf("2. View Attendance History\n");
        printf("3. Logout\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                viewSessionsForAttendance(users[i].id.PRN);
                break;
            case 2:
                viewStudentAttendanceHistory(users[i].id.PRN);
                break;
            case 3:
                logEvent("student",logTitle,"logged out");
                run = 0;
                break;
            default:
                printf("Invalid choice.\n");
                break;
        }

    }
}

void loginUser(const char *role) {
    char username[MAX_USERNAME_LEN];
    char PRN[9];
    char password[MAX_PASSWORD_LEN];

    if(strcmp(role,"student") == 0){

        printf("Enter PRN: ");
        scanf("%s", PRN);
        printf("Enter password: ");
        getPassword(password);

        for (int i = 0; i < userCount; i++) {
            printf("%s %s %s\n",PRN,users[i].id.PRN,users[i].role);
            if (strcmp(users[i].id.PRN, PRN) == 0 && strcmp(users[i].password, password) == 0 && strcmp(users[i].role, "student") == 0) {
                printf("Login successful! Welcome, %s.\n", users[i].username);
                studentMenu(i);
                return;
            }
        }
    }else{
        printf("Enter username: ");
        scanf("%s", username);
        printf("Enter password: ");
        getPassword(password);

        for (int i = 0; i < userCount; i++) {
            if (strcmp(users[i].username, username) == 0 && strcmp(users[i].password, password) == 0 && strcmp(users[i].role, role) == 0) {
                printf("Login successful! Welcome, %s.\n", username);
                teacherMenu(i);
                return;
            }
    }}

    printf("Invalid username or password.\n");
}

void createAccount() {
    if (userCount >= MAX_USERS) {
        printf("User limit reached. Cannot create a new account.\n");
        return;
    }

    User newUser;
    while (1) {
        printf("Enter role (teacher/student): ");
        scanf("%s", newUser.role);
        if (strcmp(newUser.role, "teacher") == 0 || strcmp(newUser.role, "student") == 0) {
            break;
        } else {
            printf("Invalid role. Please enter 'teacher' or 'student'.\n");
        }
    };
    printf("Enter username: ");
    scanf("%s", newUser.username);
    if(strcmp(newUser.role,"teacher") == 0){
        for (int i = 0; i < userCount; i++) {
                if (strcmp(users[i].username, newUser.username) == 0) {
                    printf("Username already exists. Try a different username.\n");
                    return;
                }
        }
    }


    printf("Enter password: ");
    getPassword(newUser.password);

    if(strcmp(newUser.role,"teacher") == 0){
        printf("Enter subject name: ");   
        scanf("%s",newUser.id.subject);
        
    }else{
        printf("Enter PRN Number: ");
        scanf("%s",newUser.id.PRN);

        for (int i = 0; i < userCount; i++) {
            if (strcmp(users[i].id.PRN, newUser.id.PRN) == 0) {
                printf("PRN already exists\n");
                return;
            }
        }
    }


    users[userCount++] = newUser;
    saveUsersToFile();
    if(strcmp(newUser.role,"teacher") == 0){
        logEvent("teacher",newUser.username,"created account");
    }else{
        char logTitle[MAX_USERNAME_LEN+MAX_PRN_LEN];
        sprintf(logTitle,"%s %s",newUser.username,newUser.id.PRN);
        logEvent("student",logTitle,"created account");
    }
    printf("Account created successfully!\n");
}

int main() {
    int choice;
    loadUsersFromFile();
    loadSessionsFromFile();
    loadAttendanceFromFile();

    while (1) {
        printf("\nSecure Attendance Tracker\n");
        printf("1. Login as Teacher\n");
        printf("2. Login as Student\n");
        printf("3. Create New Account\n");
        printf("4. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                loginUser("teacher");
                break;
            case 2:
                loginUser("student");
                break;
            case 3:
                createAccount();
                break;
            case 4:
                printf("Exiting the program...\n");
                exit(0);
                break;
            default:
                printf("Invalid choice. Try again.\n");
                break;
        }

    }

    return 0;
}