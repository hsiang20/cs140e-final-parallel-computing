#include <fmpi.h>
#include <stdio.h>
#include <stdlib.h>

// Function to merge two sorted subarrays
void merge(data_type *arr, int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;
    int L[n1], R[n2];

    for (int i = 0; i < n1; i++)
        L[i] = arr[left + i];
    for (int i = 0; i < n2; i++)
        R[i] = arr[mid + 1 + i];

    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2)
        arr[k++] = (L[i] <= R[j]) ? L[i++] : R[j++];
    
    while (i < n1) arr[k++] = L[i++];
    while (j < n2) arr[k++] = R[j++];
}

// Sequential Merge Sort
void merge_sort(data_type *arr, int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        merge_sort(arr, left, mid);
        merge_sort(arr, mid + 1, right);
        merge(arr, left, mid, right);
    }
}

void notmain() {
    // int rank, size;
    // MPI_Init(&argc, &argv);
    // MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    // MPI_Comm_size(MPI_COMM_WORLD, &size);
    uart_init();
    const uint8_t rank = *(uint8_t *)(0x8000);
    const uint8_t size = *(uint8_t *)(0x8000 + 1);
    printk("rank: %d, size: %d\n", rank, size);

    FMPI_Init(rank, size, 0);

    int N = 16; // Array size
    // data_type *arr = NULL;
    data_type arr[N];

    if (rank == 0) {
        // Root initializes array with random numbers
        // arr = (data_type *)malloc(N * sizeof(data_type));
        printk("Unsorted array: ");
        for (int i = 0; i < N; i++) {
            // arr[i] = rand() % 100;
            arr[i] = 100 - i;
            printk("%d ", arr[i]);
        }
        printk("\n");
    }

    // Broadcast array size
    // MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // int local_size = N / size; 
    int local_size = 8;
    // data_type *sub_arr = (data_type *)malloc(local_size * sizeof(data_type));
    data_type sub_arr[local_size];

    // Scatter data among processes
    // MPI_Scatter(arr, local_size, MPI_INT, sub_arr, local_size, MPI_INT, 0, MPI_COMM_WORLD);
    FMPI_Scatter(arr, local_size,
        sub_arr, local_size);

    // Each process sorts its local chunk
    merge_sort(sub_arr, 0, local_size - 1);

    // Gather sorted chunks back to root
    // MPI_Gather(sub_arr, local_size, MPI_INT, arr, local_size, MPI_INT, 0, MPI_COMM_WORLD);
    FMPI_Gather(sub_arr, local_size,
        arr, local_size);


    if (rank == 0) {
        // Root merges sorted subarrays
        for (int i = 1; i < size; i++) {
            int mid = (i * local_size) - 1;
            merge(arr, 0, mid, (i + 1) * local_size - 1);
        }

        printk("Sorted array: ");
        for (int i = 0; i < N; i++)
            printk("%d ", arr[i]);
        printk("\n");

        // free(arr);
    }

    // free(sub_arr);
    // MPI_Finalize();
    output("rank: %d finished\n", rank);
}