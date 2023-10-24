#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include<stdbool.h>  
#include <math.h>
int count;
float time_slice;
float time_slice1;
float time_slice2;
float time_slice3;
float boost_time;
struct Process {
    char pid[10];
    float arrival_time;
    float burst_time;
    float remaining_time;
    float first_schedule_time;
    int current_queue;
};

struct Process processes[10000];


int compareArrivalTime(const void *a, const void *b) {
    struct Process *p1 = (struct Process *)a;
    struct Process *p2 = (struct Process *)b;
    return (p1->arrival_time >= p2->arrival_time) - (p1->arrival_time < p2->arrival_time);
}
struct MinHeap {
    struct Process* array;
    int capacity;
    int size;
};

void swap(struct Process* a, struct Process* b) {
    struct Process temp = *a;
    *a = *b;
    *b = temp;
}

struct MinHeap* createMinHeap(int capacity) {
    struct MinHeap* minHeap = (struct MinHeap*)malloc(sizeof(struct MinHeap));
    minHeap->array = (struct Process*)malloc(sizeof(struct Process) * capacity);
    minHeap->capacity = capacity;
    minHeap->size = 0;
    return minHeap;
}

void heapify(struct MinHeap* minHeap, int i, int attribute) {
    int smallest = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;
    if(attribute == 1){
    if (left < minHeap->size && minHeap->array[left].remaining_time < minHeap->array[smallest].remaining_time) {
        smallest = left;
    }

    if (right < minHeap->size && minHeap->array[right].remaining_time < minHeap->array[smallest].remaining_time) {
        smallest = right;
    }
    }
    else {
       if (left < minHeap->size && minHeap->array[left].burst_time < minHeap->array[smallest].burst_time) {
        smallest = left;
    }

    if (right < minHeap->size && minHeap->array[right].burst_time < minHeap->array[smallest].burst_time) {
        smallest = right;
    } 
    }

    if (smallest != i) {
        swap(&minHeap->array[i], &minHeap->array[smallest]);
        heapify(minHeap, smallest, attribute);
    }
}

void insert(struct MinHeap* minHeap, struct Process process) {
    if (minHeap->size == minHeap->capacity) {
        printf("Heap is full, cannot insert more elements.\n");
        return;
    }

    int i = minHeap->size;
    minHeap->array[i] = process;
    minHeap->size++;

    while (i != 0 && minHeap->array[i].remaining_time < minHeap->array[(i - 1) / 2].remaining_time) {
        swap(&minHeap->array[i], &minHeap->array[(i - 1) / 2]);
        i = (i - 1) / 2;
    }
}

struct Process extractMin(struct MinHeap* minHeap, int attribute) {
    struct Process minProcess;

    if (minHeap->size <= 0) {
        minProcess.burst_time = -1.0; // Invalid burst time
        minProcess.remaining_time = -1.0; // Invalid remaining time
        return minProcess;
    }

    if (minHeap->size == 1) {
        minHeap->size--;
        return minHeap->array[0];
    }

    minProcess = minHeap->array[0];
    minHeap->array[0] = minHeap->array[minHeap->size - 1];
    minHeap->size--;
    heapify(minHeap, 0, attribute);
    return minProcess;
}
void SRTF_Scheduling(struct Process* processes, int count, const char* outputFileName){
    FILE* fp = fopen(outputFileName, "a");
    if (fp == NULL) {
        printf("Error opening file\n");
        exit(1);
    }
    float current_time = 0;
    int completed = 0;
    struct MinHeap* minHeap = createMinHeap(count);
    int i = 0;
    float total_turnaround_time = 0;
    float total_response_time = 0;
    while (completed != count) {
        while (i < count && processes[i].arrival_time <= current_time) {
            insert(minHeap, processes[i]);
            i++;
        }
        if (minHeap->size == 0) {
            current_time = processes[i].arrival_time;
            continue;
        }
        struct Process minProcess = extractMin(minHeap, 1);
        if (minProcess.first_schedule_time == -1) {
            minProcess.first_schedule_time = current_time;
        }
        // minProcess.remaining_time -= 0.1;
        // current_time += 0.1;
        if(minHeap->size == 0){
            current_time += minProcess.remaining_time;
            minProcess.remaining_time = 0;
        }
        else{
            struct Process nextProcess = minHeap->array[0];
            if(nextProcess.remaining_time < minProcess.remaining_time){
                fprintf(fp, "%s %0.1f %0.1f ", minProcess.pid, current_time, current_time + nextProcess.remaining_time);
                current_time += nextProcess.remaining_time;
                minProcess.remaining_time -= nextProcess.remaining_time;
            }
            else{
                fprintf(fp, "%s %0.1f %0.1f ", minProcess.pid, current_time, current_time + minProcess.remaining_time);
                current_time+=minProcess.remaining_time;
                minProcess.remaining_time = 0;
            }
        }
        if(minProcess.remaining_time == 0) {
            completed++;
            total_turnaround_time += current_time - minProcess.arrival_time;
            total_response_time += minProcess.first_schedule_time - minProcess.arrival_time;
            //printf("Process ID: %s, Arrival Time: %.1f, Burst Time: %.1f, Response Time: %.1f, Turnaround Time: %.1f\n", minProcess.pid, minProcess.arrival_time, minProcess.burst_time, minProcess.first_schedule_time - minProcess.arrival_time, current_time - minProcess.arrival_time);
        }
        else {
            insert(minHeap, minProcess);
        }
    }
    fprintf(fp, "\n%.2f ", total_turnaround_time / count);
    fprintf(fp, "%.2f\n", total_response_time / count);
    free(minHeap->array);
    free(minHeap);
}
void SJF_Scheduling(struct Process processes[], int count, const char* outputFileName) {
    
    FILE* fp = fopen(outputFileName, "a");
    if (fp == NULL) {
        printf("Error opening file\n");
        exit(1);
    }
    float current_time = 0;
    float total_turnaround_time = 0;
    float total_response_time = 0;

    // printf("SJF Scheduling:\n");
    // printf("Time\tPID\tArrival Time\tBurst Time\tTurnaround Time\tResponse Time\n");

    int completed_processes = 0;
    int current_process = 0;

    struct MinHeap* minHeapBurstTime = createMinHeap(count);

    while (completed_processes < count) {
        // Enqueue arriving processes during execution
        while (current_process < count && processes[current_process].arrival_time <= current_time) {
            insert(minHeapBurstTime, processes[current_process]);
            current_process++;
        }

        if (minHeapBurstTime->size == 0) {
            current_time = processes[current_process].arrival_time;
            continue;
        }

        
        struct Process selected_process = extractMin(minHeapBurstTime, 2);

        

        float turnaround_time = current_time + selected_process.burst_time - selected_process.arrival_time;
        float response_time = current_time - selected_process.arrival_time;

        total_turnaround_time += turnaround_time;
        total_response_time += response_time;

        fprintf(fp, "%s %0.1f %0.1f ", selected_process.pid, current_time, current_time + selected_process.burst_time);
        current_time += selected_process.burst_time;
        completed_processes++;
    }

    float average_turnaround_time = total_turnaround_time / count;
    float average_response_time = total_response_time / count;

    fprintf(fp, "\n%.2f ", average_turnaround_time);
    fprintf(fp, "%.2f\n", average_response_time);

    free(minHeapBurstTime->array);
    free(minHeapBurstTime);
}


void FCFS_Scheduling(struct Process processes[], int count, const char* outputFileName) {
    float current_time = 0;
    float total_turnaround_time = 0;
    float total_response_time = 0;

    FILE* fp = fopen(outputFileName, "w");
    if (fp == NULL) {
        printf("Error opening file\n");
        exit(1);
    }

    for (int i = 0; i < count; i++) {
        if (current_time < processes[i].arrival_time) {
            current_time = processes[i].arrival_time;
        }

        fprintf(fp, "%s %0.1f %0.1f ", processes[i].pid, current_time, current_time + processes[i].burst_time);

        float turnaround_time = current_time + processes[i].burst_time - processes[i].arrival_time;
        float response_time = current_time - processes[i].arrival_time;

        total_turnaround_time += turnaround_time;
        total_response_time += response_time;
        current_time += processes[i].burst_time;
    }

    float average_turnaround_time = total_turnaround_time / count;
    float average_response_time = total_response_time / count;

    fprintf(fp, "\n%.2f ", average_turnaround_time);
    fprintf(fp, "%.2f\n", average_response_time);
}

void RR_Scheduling(struct Process processes[], int count, const char* outputFileName) {
        FILE* fp = fopen(outputFileName, "a");
    if (fp == NULL) {
        printf("Error opening file\n");
        exit(1);
    }
    
    float time_slice =  5.0;
    float current_time = 0;
    float total_turnaround_time = 0;
    float total_response_time = 0;
    bool flag=true;
    while(flag)
    {
        flag = 0;
        for (int i = 0; i < count; i++)
        {
            if (current_time < processes[i].arrival_time) {
                current_time = processes[i].arrival_time;
            }
            if(processes[i].first_schedule_time == -1)processes[i].first_schedule_time = current_time;
            if(processes[i].remaining_time <= 0)continue;
            do{
                if(processes[i].remaining_time <= time_slice)
                {

                    float turnaround_time = current_time + processes[i].remaining_time - processes[i].arrival_time;
                    float response_time = processes[i].first_schedule_time - processes[i].arrival_time;
                    total_turnaround_time += turnaround_time;
                    total_response_time += response_time;
                    fprintf(fp, "%s %0.1f %0.1f ", processes[i].pid, current_time, current_time + processes[i].remaining_time);
                    current_time+=processes[i].remaining_time;
                    processes[i].remaining_time = 0;

                }else{
                    fprintf(fp, "%s %0.1f %0.1f ", processes[i].pid, current_time, current_time + time_slice);
                    current_time+=time_slice;
                    processes[i].remaining_time -= time_slice;
                    flag = 1;
                }
            } while((i+1) < count && current_time < processes[i+1].arrival_time && processes[i].remaining_time!=0);
        }
        
    }

    float average_turnaround_time = total_turnaround_time / count;
    float average_response_time = total_response_time / count;

    fprintf(fp, "\n%.2f ", average_turnaround_time);
    fprintf(fp, "%.2f\n", average_response_time);
}

struct ProcessQueue{
    struct Process* queue[10];//count here
    int front, rear;
};
void init(struct ProcessQueue* q){
    q->front = -1;
    q->rear = -1;
}
bool isEmpty(struct ProcessQueue* q){
    return q->front == -1;
}
bool isFull(struct ProcessQueue* q){
    return q->front == (q->rear + 1) % count;
}
void enqueue(struct ProcessQueue* q, struct Process* process){
    if(isFull(q)){
        printf("Queue is full\n");
        return;
    }
    if(isEmpty(q)){
        q->front = 0;
        q->rear = 0;
    }
    else{
        q->rear = (q->rear + 1) % count;
    }
    q->queue[q->rear] = process;
}
struct Process* dequeue(struct ProcessQueue* q){
    if(isEmpty(q)){
        printf("Queue is empty\n");
        return NULL;
    }
    struct Process* process = q->queue[q->front];
    if(q->front == q->rear){
        q->front = -1;
        q->rear = -1;
    }
    else{
        q->front = (q->front + 1) % count;
    }
    return process;
}
void Boost(struct ProcessQueue* queues){
    for(int i=1;i<3;i++){
        while(!isEmpty(&queues[i])){
            struct Process* boosted_process = dequeue(&queues[i]);
            enqueue(&queues[0], boosted_process);
            boosted_process->current_queue = 0;
        }
    }
}

void MLFQ_Scheduling(struct Process processes[], int count, const char* outputFileName){
    FILE* fp = fopen(outputFileName, "a");
    if (fp == NULL) {
        printf("Error opening file\n");
        exit(1);
    }
    float current_time = 0;
    float help;
    float total_turnaround_time = 0;
    float total_response_time = 0;
    int completed=0;
    struct ProcessQueue queues[3];
    for(int i=0;i<3;i++){
        init(&queues[i]);
    }
    int curr=0;
    float boostcount=1;
    while(completed != count){
        help=current_time;
          while(curr < count && processes[curr].arrival_time <= current_time){
            enqueue(&queues[0], &processes[curr]);
            curr++;
        }
        struct Process* current_process = NULL;
       
        for(int i=0;i<3;i++){
            if(!isEmpty(&queues[i])){
                current_process = dequeue(&queues[i]);
                break;
            }
        }
        if(current_process != NULL){
           
            if(current_process->first_schedule_time == -1){
                current_process->first_schedule_time = current_time;
            }
            float tslice=0;
            if(current_process->current_queue == 0){
                tslice = time_slice1;
            }
            else if(current_process->current_queue == 1){
                tslice = time_slice2;
            }
            else{
                tslice = time_slice3;
            }
            if(current_process->remaining_time <= tslice){
                // bool flag= false;
                fprintf(fp, "%s %0.1f %0.1f ", current_process->pid, current_time, current_time + current_process->remaining_time);
                current_time+=current_process->remaining_time;
                //print current time only
                

                // if(current_time >= boostcount*boost_time && boost_time != 0){
                //     current_time=boostcount*boost_time;
                //     current_process->remaining_time -= current_time - help;
                //     if(current_process->remaining_time == 0) completed++;
                //     Boost(queues);
                //     boostcount++;
                //     flag=true;
                // }
                // while(curr < count && processes[curr].arrival_time <= current_time){
                //     enqueue(&queues[0], &processes[curr]);
                //     curr++;
                // }
                // if(flag) continue;
                float turnaround_time = current_time - current_process->arrival_time;
                float response_time = current_process->first_schedule_time - current_process->arrival_time;
                total_turnaround_time += turnaround_time;
                total_response_time += response_time;
                current_process->remaining_time = 0;
                completed++;
            }
            else{
                // bool flag=false;
                fprintf(fp, "%s %0.1f %0.1f ", current_process->pid, current_time, current_time + tslice);
                current_time+=tslice;
                // if(current_time >= boostcount*boost_time && boost_time != 0){
                //     current_time=boostcount*boost_time;
                //     current_process->remaining_time -= current_time - help;
                //     Boost(queues);
                //     boostcount++;
                //     flag=true;
                // }
                // while(curr < count && processes[curr].arrival_time <= current_time){
                //     enqueue(&queues[0], &processes[curr]);
                //     curr++;
                // }
                // if(flag) continue;
                current_process->remaining_time -= tslice;
                if(current_process->current_queue == 0){
                    current_process->current_queue = 1;
                }
                else if(current_process->current_queue == 1){
                    current_process->current_queue = 2;
                }
                enqueue(&queues[current_process->current_queue], current_process);
            }
        }
        else{
            current_time=processes[curr].arrival_time;
        }
    }
    float average_turnaround_time = total_turnaround_time / count;
    float average_response_time = total_response_time / count;
    fprintf(fp, "\n%.2f ", average_turnaround_time);
    fprintf(fp, "%.2f\n", average_response_time);
}
void renew(struct Process processes[], int count){
    for(int i=0;i<count;i++){
        processes[i].remaining_time = processes[i].burst_time;
        processes[i].first_schedule_time = -1;
    }
}
void generateProcesses(char* filename) {
    FILE *fp;
    fp= fopen(filename, "r");
    if (fp == NULL) {
        printf("Error opening file\n");
        exit(1);
    }
    int i=0;
    while(!feof(fp)){
        fscanf(fp, "%s %f %f", processes[i].pid, &processes[i].arrival_time, &processes[i].burst_time);
        processes[i].remaining_time = processes[i].burst_time;
        processes[i].first_schedule_time = -1;
        i++;
    }
    count=i;
    fclose(fp);
}


int main() {
    //input the file name
    char input[20];
    scanf("%s", input);
    char output[20];
    scanf("%s", output);
    generateProcesses(input);
    scanf("%f %f %f %f %f", &time_slice, &time_slice1, &time_slice2, &time_slice3, &boost_time);
    
    qsort(processes, count, sizeof(struct Process), compareArrivalTime);
    
    FCFS_Scheduling(processes, count, output);
    RR_Scheduling(processes, count, output);
    renew(processes, count);
    SJF_Scheduling(processes, count, output);
    renew(processes, count);
    SRTF_Scheduling(processes, count, output);
    renew(processes, count);
    MLFQ_Scheduling(processes, count, output);
    return 0;
}
