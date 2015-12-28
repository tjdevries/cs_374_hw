TARGET = vectorHypot
CC     = g++
CFLAGS = -Wall -ansi -pedantic -std=c99
LFLAGS = -o $(TARGET) -lgomp
#LFLAGS = -o $(TARGET) -L/usr/local/cuda/lib64 -lcuda -lcudart -lgomp
NVCC = nvcc
CUDA_FLAGS = -gencode arch=compute_52,code=sm_52 

$(TARGET): $(TARGET).cu
	$(NVCC) $(CUDA_FLAGS) $(TARGET).cu $(LFLAGS)

clean: 
	rm -rf *o $(TARGET)
