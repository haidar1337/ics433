#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Student {
    int id;
    char name[50];
    float gpa;
};


/*
    This function reads student records from the given file input,
    and stores the student structures in the students array variable
    outputting 0 if the function reads the file successfully or
    terminates the program if an error occurs.
*/
int readStudentRecords(const char *filename, struct Student students[], int *numStudents) {
    FILE *fp = fopen(filename, "r");
    if(fp == NULL)
    {
        fprintf(stderr, "Error: studentData.txt not found!\n");
        exit(1);
    }

    int count = 0;
    while(1)
    {
        int id;
        char name[50];
        float gpa;

        int scanned = fscanf(fp, "%d %s %f", &id, name, &gpa);
        if(scanned == EOF) // End of file reached.
        { 
            break;  
        }
        if(scanned != 3) // A value is missing.
        { 
            fprintf(stderr, "Error: The studentData.txt file is corrupted!\n");
            fclose(fp);
            exit(1);
        }
        if(count >= 10) // More than 10 students in the file.
        { 
            fprintf(stderr, "Error: Maximum student capacity reached!\n");
            fclose(fp);
            exit(1);
        }

        students[count].id = id;
        strncpy(students[count].name, name, 49);
        students[count].name[49] = '\0';
        students[count].gpa = gpa;

        count++;
    }

    fclose(fp);
    *numStudents = count;
    return 0;
}

/*
    This function displays every student record in the array to the screen.
*/

void displayStudentRecords(struct Student students[], int numStudents) {
    printf("\nStudent Records:\n");
    for(int i = 0; i < numStudents; i++)
    {
        printf("id: %d, name: %s, gpa: %.2f\n", students[i].id, students[i].name, students[i].gpa);
    }
}

/*
    This function writes a in student record to the file given as inpput
    outputting 0 if the write was successful or 1 otherwise.
*/
int writeStudentRecord(const char *filename, struct Student *student) {
    FILE *fp = fopen(filename, "a");
    if(fp == NULL)
    {
        return 1;
    }
    // student->id syntax is equivalent to (*student).id
    fprintf(fp, "%d %s %.2f\n", student->id, student->name, student->gpa);
    fclose(fp);
    return 0;
}


/*
    The main function stores the necessary variaables and takes command line arguments
    validates the inputs and does the required functionality by invoking the other functions.
*/
int main(int argc, char *argv[]) {
    struct Student students[10];
    int numStudents = 0;
    const char *filename = "studentData.txt";
    
    readStudentRecords(filename, students, &numStudents);
    
    if(numStudents >= 10)
    {
        fprintf(stderr, "Error: Maximum student capacity reached!\n");
        return 1;
    }
    
    if(argc != 4)
    {
        fprintf(stderr, "Usage: %s studentID name GPA\n", argv[0]);
        return 1;
    }
    
    int inID = atoi(argv[1]);
    

    char *endPtr;
    float inGPA = strtof(argv[3], &endPtr);
    if(*endPtr != '\0')
    {
        fprintf(stderr, "Error: Invalid GPA format.\n");
        return 1;
    }
    if(inGPA < 0.0 || inGPA > 4.0)
    {
        fprintf(stderr, "Error: GPA must be between 0.0 and 4.0 inclusive.\n");
        return 1;
    }
    
    // check for duiplication
    for(int i = 0; i < numStudents; i++)
    {
        if(students[i].id == inID)
        {
            fprintf(stderr, "Error: Student ID already exists!\n");
            return 1;
        }
    }
    
    struct Student in;
    in.id = inID;
    strncpy(in.name, argv[2], 49);
    in.name[49] = '\0';
    in.gpa = inGPA;
    
    students[numStudents] = in;
    numStudents++;
    
    displayStudentRecords(students, numStudents);
    
    if(writeStudentRecord(filename, &in) != 0)
    {
        fprintf(stderr, "Error: Failed to write student data to the file.\n");
        return 1;
    }
    
    return 0;
}
