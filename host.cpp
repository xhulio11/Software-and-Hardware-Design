
#include "xcl2.hpp"
#include <algorithm>
#include <vector>
#include "header.h"

std::vector<int, aligned_allocator<int>> RandomGenerator(int size){
	std::vector<int, aligned_allocator<int>> array(size);
	for(int i = 0; i < size; i++){
		array[i] = std::rand()%10000;
	}
	return array;
}

void printer(std::vector<int, aligned_allocator<int>> source_in1){
	for(int i = 0; i < source_in1.size() ; i++){
		if(!(i%n)){
			std::cout<<std::endl;
		}
		std::cout<<source_in1[i]<<" ";
	}
	std::cout<<"\n"<<"\n";
}

std::vector<int, aligned_allocator<int>> mmsw(std::vector<int, aligned_allocator<int>> source_in1, std::vector<int, aligned_allocator<int>> source_in2,
	std::vector<int, aligned_allocator<int>>source_sw_results){

    for(int i = 0; i < n; i++){
        for(int j = 0; j < p; j++){
            int sum = 0;
            for(int k = 0; k < m  ; k++){
                sum += source_in1[i*m + k] * source_in2[k*p + j];

            }
            source_sw_results[i*p + j] = sum;
        }
    }
    return source_sw_results;
}


bool tester(std::vector<int, aligned_allocator<int>> source_sw_results, std::vector<int, aligned_allocator<int>> source_hw_results){
	for(int i = 0; i < n; i++){
		for(int j = 0; j < p; j++){
			if(source_sw_results[i*p + j] != source_hw_results[i*p + j] )return false;
		}
	}
	return true;
}


int main(int argc, char **argv) {
  if (argc != 2) {
    std::cout << "Usage: " << argv[0] << " <XCLBIN File>" << std::endl;
    return EXIT_FAILURE;
  }

  std::string binaryFile = argv[1];

  //size_t vector_size_bytes = sizeof(int) * DATA_SIZE;

  size_t vector_size_in1 = n*m*sizeof(int);
  size_t vector_size_in2 = m*p*sizeof(int);
  size_t vector_size_hw_results = n*p*sizeof(int);

  cl_int err;
  cl::Context context;
  cl::Kernel krnl_matrix_mul;
  cl::CommandQueue q;
  // Allocate Memory in Host Memory
  // When creating a buffer with user pointer (CL_MEM_USE_HOST_PTR), under the
  // hood user ptr
  // is used if it is properly aligned. when not aligned, runtime had no choice
  // but to create
  // its own host side buffer. So it is recommended to use this allocator if
  // user wish to
  // create buffer using CL_MEM_USE_HOST_PTR to align user buffer to page
  // boundary. It will
  // ensure that user buffer is used when user create Buffer/Mem object with
  // CL_MEM_USE_HOST_PTR

  std::vector<int, aligned_allocator<int>> source_in1(n*m);
  std::vector<int, aligned_allocator<int>> source_in2(m*p);
  source_in1 = RandomGenerator(n*m);
  source_in2 = RandomGenerator(m*p);
  std::vector<int, aligned_allocator<int>> source_hw_results(n*p);
  std::vector<int, aligned_allocator<int>> source_sw_results(n*p);

  std::cout<<"Source 1";
  printer(source_in1);
  std::cout<<"Source 2";
  printer(source_in2);

  for(unsigned int j = 0; j < source_hw_results.size(); j++){
	  source_hw_results[j] = 0;
  }
  std::cout<<"Source HW Result with 0.";
  printer(source_hw_results);


  // Create the test data
  /*
  for(int i = 0; i < n; i++){
	  std::generate(source_in1[i].begin(), source_in1[i].end(), std::rand);
  }
  for(int i = 0; i < m; i++){
	  std::generate(source_in2[i].begin(), source_in2[i].end(), std::rand);
  }

  for(unsigned int i = 0; i < source_hw_results.size(); i++){
	  for(unsigned int j = 0; j < source_hw_results[0].size(); j++){
		  source_hw_results[i][j] = 0;
	  }
  }
  */
  // Running multiplication for software
  source_sw_results = mmsw(source_in1,source_in2,source_sw_results);


  // ---------------------------------------------------------------------------
  // OPENCL HOST CODE AREA START
  // get_xil_devices() is a utility API which will find the xilinx
  // platforms and will return list of devices connected to Xilinx platform
  auto devices = xcl::get_xil_devices();
  // read_binary_file() is a utility API which will load the binaryFile
  // and will return the pointer to file buffer.
  auto fileBuf = xcl::read_binary_file(binaryFile);
  cl::Program::Binaries bins{{fileBuf.data(), fileBuf.size()}};
  int valid_device = 0;
  for (unsigned int i = 0; i < devices.size(); i++) {
    auto device = devices[i];
    // Creating Context and Command Queue for selected Device
    OCL_CHECK(err, context = cl::Context(device, NULL, NULL, NULL, &err));
    OCL_CHECK(err, q = cl::CommandQueue(context, device,
                                        CL_QUEUE_PROFILING_ENABLE, &err));
    //std::cout << "Trying to program device[" << i
              //<< "]: " << device.getInfo<CL_DEVICE_NAME>() << std::endl;
    cl::Program program(context, {device}, bins, NULL, &err);
    if (err != CL_SUCCESS) {
      //std::cout << "Failed to program device[" << i << "] with xclbin file!\n";
    } else {
      //std::cout << "Device[" << i << "]: program successful!\n";
      OCL_CHECK(err, krnl_matrix_mul = cl::Kernel(program, "vadd", &err));
      valid_device++;
      break; // we break because we found a valid device
    }
  }
  if (valid_device == 0) {
    std::cout << "Failed to program any device found, exit!\n";
    exit(EXIT_FAILURE);
  }
  // ------------------------------------------------------------------------

  // Allocate Buffer in Global Memory
  // Buffers are allocated using CL_MEM_USE_HOST_PTR for efficient memory and
  // Device-to-host communication
  OCL_CHECK(err, cl::Buffer buffer_in1(
                     context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
                     vector_size_in1, source_in1.data(), &err));
  OCL_CHECK(err, cl::Buffer buffer_in2(
                     context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
                     vector_size_in2, source_in2.data(), &err));
  OCL_CHECK(err, cl::Buffer buffer_output(
                     context, CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY,
                     vector_size_hw_results, source_hw_results.data(), &err));

  OCL_CHECK(err, err = krnl_matrix_mul.setArg(0, buffer_in1));
  OCL_CHECK(err, err = krnl_matrix_mul.setArg(1, buffer_in2));
  OCL_CHECK(err, err = krnl_matrix_mul.setArg(2, buffer_output));

  // Copy input data to device global memory
  OCL_CHECK(err, err = q.enqueueMigrateMemObjects({buffer_in1, buffer_in2},
                                                  0 /* 0 means from host*/));

  // Launch the Kernel
  // For HLS kernels global and local size is always (1,1,1). So, it is
  // recommended
  // to always use enqueueTask() for invoking HLS kernel
  OCL_CHECK(err, err = q.enqueueTask(krnl_matrix_mul));

  // Copy Result from Device Global Memory to Host Local Memory
  OCL_CHECK(err, err = q.enqueueMigrateMemObjects({buffer_output},
                                                  CL_MIGRATE_MEM_OBJECT_HOST));
  q.finish();
  // OPENCL HOST CODE AREA END

/*  // Compare the results of the Device to the simulation
  bool match = true;
  for (int i = 0; i < DATA_SIZE; i++) {
    if (source_hw_results[i] != source_sw_results[i]) {
      std::cout << "Error: Result mismatch" << std::endl;
      std::cout << "i = " << i << " CPU result = " << source_sw_results[i]
                << " Device result = " << source_hw_results[i] << std::endl;
      match = false;
      break;
    }
  }
  std::cout << "TEST " << (match ? "PASSED" : "FAILED") << std::endl;
  return (match ? EXIT_SUCCESS : EXIT_FAILURE);
*/
  std::cout<<"Source SW Results";
  printer(source_sw_results);

  std::cout<<"Source HW Results";
  printer(source_hw_results);

  if (tester(source_sw_results, source_hw_results)){
	  std::cout<<"TEST PASSED"<<std::endl;
  }else{
	  std::cout<<"TEST FAILED"<<std::endl;
  }

}
