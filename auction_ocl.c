#include <stdio.h>
#include <stdlib.h>

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#define MAX_SOURCE_SIZE (0x100000)
#define NODES 1500
#define ARCS 3600
#define INF	999999

int main()
{
    cl_platform_id platform_id = NULL;
    cl_device_id device_id = NULL;
    cl_context context = NULL;
    cl_command_queue command_queue = NULL;
    cl_mem FPRmobj = NULL;
    cl_mem NETmobj = NULL;
    cl_mem NETImobj = NULL;
    cl_mem PRImobj = NULL;
    cl_mem Pmobj = NULL;
    cl_program program = NULL;
    cl_kernel kernel[NODES];
    cl_uint ret_num_devices;
    cl_uint ret_num_platforms;
    cl_int ret;

    int i, j;
    int *fpr;
    int *prices, *P, (*network)[2], (*network_i)[2];

    for (i = 0; i < NODES; i++) {
    	kernel[i] = NULL;
    }

    /* alokacja pamieci dla tablic */
    fpr = (int*)malloc(NODES*sizeof(int));
	network = (int (*)[2])malloc((ARCS+NODES)*2*sizeof(int));
	network_i = (int (*)[2])malloc((ARCS+NODES)*2*sizeof(int));
	prices = (int*)malloc((NODES+1)*sizeof(int));
	P = (int*)malloc((NODES+1)*sizeof(int));

	int source, tail, nodes, arcs;
	read_network("outp", &source, &tail, &nodes, &arcs, network, network_i);

    FILE *fp;
    const char fileName[] = "./auction_ocl.cl";
    size_t source_size;
    char *source_str;

    /* ladowanie pliku kernela i przydzial pamieci */
    fp = fopen(fileName, "rb");
    if (!fp) {
        fprintf(stderr, "Failed to load kernel.\n");
        exit(1);
    }
    source_str = (char *)malloc(MAX_SOURCE_SIZE);
    source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
    fclose(fp);

    /* inicjalizacja danych poczatkowych tablic */
    for (i = 0; i < NODES; i++) {
    	fpr[i] = INF;
		P[i] = INF;
		pr[i] = 0;
    }

    /* pobranie informacji o srodowisku */
    ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
    ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_DEFAULT, 1, &device_id, &ret_num_devices);

    /* utworzenie kontekstu */
    context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);

    /* utworzenie kolejki */
    command_queue = clCreateCommandQueue(context, device_id, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, &ret);

    /* utworzenie buforow */
    FPRmobj = clCreateBuffer(context, CL_MEM_READ_WRITE, NODES*sizeof(float), NULL, &ret);

    /* skopiowanie danych wejsciowych - tablicy fpr - do bufora */
    ret = clEnqueueWriteBuffer(command_queue, FPRmobj, CL_TRUE, 0, NODES*sizeof(float), fpr, 0, NULL, NULL);
    ret = clEnqueueWriteBuffer(command_queue, NETmobj, CL_TRUE, 0, (ARCS+NODES)*2*sizeof(float), network, 0, NULL, NULL);
    ret = clEnqueueWriteBuffer(command_queue, NETImobj, CL_TRUE, 0, (ARCS+NODES)*2*sizeof(float), network_i, 0, NULL, NULL);
    ret = clEnqueueWriteBuffer(command_queue, PRImobj, CL_TRUE, 0, NODES*sizeof(float), prices, 0, NULL, NULL);
    ret = clEnqueueWriteBuffer(command_queue, Pmobj, CL_TRUE, 0, NODES*sizeof(float), P, 0, NULL, NULL);

    /* utworzenie kernela */
    program = clCreateProgramWithSource(context, 1, (const char **)&source_str, (const size_t *)&source_size, &ret);
    ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);

    for (i = 0; i < NODES; i++) {
    	kernel[0] = clCreateKernel(program, "singleAuctionSearch", &ret);
    }

    /* przekazanie argumentow */
    for (i = 0; i < NODES; i++) {
        ret = clSetKernelArg(kernel[i], 0, sizeof(cl_mem), (void *)&FPRmobj);
        ret = clSetKernelArg(kernel[i], 0, sizeof(cl_mem), (void *)&NETmobj);
        ret = clSetKernelArg(kernel[i], 0, sizeof(cl_mem), (void *)&NETImobj);
        ret = clSetKernelArg(kernel[i], 0, sizeof(cl_mem), (void *)&PRImobj);
        ret = clSetKernelArg(kernel[i], 0, sizeof(cl_mem), (void *)&Pmobj);
    }

    /* uruchomienie kernelow jako zadania rownolegle */
    for (i = 0; i < NODES; i++) {
        ret = clEnqueueTask(command_queue, kernel[i], 0, NULL, NULL);
    }

    /* skopiowanie wyniku do hosta */
    ret = clEnqueueReadBuffer(command_queue, FPRmobj, CL_TRUE, 0, NODES*sizeof(float), C, 0, NULL, NULL);

    /* wyswietlenie wyniku */
/*    for (i=0; i < 4; i++) {
        for (j=0; j < 4; j++) {
            printf("%7.2f ", C[i*4+j]);
        }
        printf("\n");
    }
*/
    /* czynnosci koncowe */
    ret = clFlush(command_queue);
    ret = clFinish(command_queue);

    for (i = 0; i < NODES; i++) {
    	ret = clReleaseKernel(kernel[i]);
    }

    ret = clReleaseProgram(program);

    ret = clReleaseMemObject(FPRmobj);
    ret = clReleaseMemObject(NETmobj);
    ret = clReleaseMemObject(NETImobj);
    ret = clReleaseMemObject(PRImobj);
    ret = clReleaseMemObject(Pmobj);

    ret = clReleaseCommandQueue(command_queue);
    ret = clReleaseContext(context);

    free(source_str);

    free(fpr);

    return 0;
}
