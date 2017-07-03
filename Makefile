all:
	g++ -c test_utils.cpp -L/usr/lib/x86_64-linux-gnu -lavcodec -lswscale -lavutil -lavformat -o test_utils.o

	g++ -c AlphaCompC_8u_C1R_test.cpp -I/opt/intel/ipp/include -o AlphaCompC_8u_C1R_test.o

	g++ -c Resize_8u_C1R_test.cpp -I/opt/intel/ipp/include -o Resize_8u_C1R_test.o

	g++ -c Set_8u_C1R_test.cpp -I/opt/intel/ipp/include  -o Set_8u_C1R_test.o

	g++ -c RGBToYCbCr_8u_P3R_test.cpp -I/opt/intel/ipp/include  -o RGBToYCbCr_8u_P3R_test.o

	g++ -c YCbCrToRGB_8u_P3R_test.cpp -I/opt/intel/ipp/include  -o YCbCrToRGB_8u_P3R_test.o

	g++ -c Mirror_8u_C1IR_test.cpp -I/opt/intel/ipp/include  -o Mirror_8u_C1IR_test.o

	g++ -c Mirror_8u_C1R_test.cpp -I/opt/intel/ipp/include  -o Mirror_8u_C1R_test.o

	g++ -c Copy_8u_C1R_test.cpp -I/opt/intel/ipp/include  -o Copy_8u_C1R_test.o

	g++ -c Copy_8u_C4C1R_test.cpp -I/opt/intel/ipp/include -o Copy_8u_C4C1R_test.o

	g++ -c Copy_8u_C3P3R_test.cpp -I/opt/intel/ipp/include  -o Copy_8u_C3P3R_test.o

	g++ -c unittest1.cpp  -I/opt/intel/ipp/include  -o unittest1.o

	g++ unittest1.o YCbCrToRGB_8u_P3R_test.o Copy_8u_C4C1R_test.o AlphaCompC_8u_C1R_test.o Set_8u_C1R_test.o RGBToYCbCr_8u_P3R_test.o Mirror_8u_C1IR_test.o Mirror_8u_C1R_test.o Copy_8u_C1R_test.o Copy_8u_C3P3R_test.o Resize_8u_C1R_test.o test_utils.o  -I/opt/intel/ipp/include -L/opt/intel/ipp/lib/intel64_lin -lippcc -lippcore -lippi -L/usr/lib/x86_64-linux-gnu -lavcodec -lswscale -lavutil -lavformat -o a
