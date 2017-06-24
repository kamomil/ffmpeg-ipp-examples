all:
	g++ -c test_utils.cpp -L/usr/lib/x86_64-linux-gnu -lavcodec -lswscale -lavutil -lavformat -o test_utils.o

	g++ -c RGBToYCbCr_8u_P3R_test.cpp -I/opt/intel/ipp/include -L/opt/intel/ipp/lib/intel64_lin -lippcc -lippcore -lippi -L/usr/lib/x86_64-linux-gnu -lavcodec -lswscale -lavutil -lavformat -o RGBToYCbCr_8u_P3R_test.o

	g++ -c Copy_8u_C1R_test.cpp -I/opt/intel/ipp/include -L/opt/intel/ipp/lib/intel64_lin -lippcc -lippcore -lippi -L/usr/lib/x86_64-linux-gnu -lavcodec -lswscale -lavutil -lavformat -o Copy_8u_C1R_test.o

	g++ -c Copy_8u_C3P3R_test.cpp -I/opt/intel/ipp/include -L/opt/intel/ipp/lib/intel64_lin -lippcc -lippcore -lippi -L/usr/lib/x86_64-linux-gnu -lavcodec -lswscale -lavutil -lavformat -o Copy_8u_C3P3R_test.o

	g++ -c unittest1.cpp  -I/opt/intel/ipp/include -L/opt/intel/ipp/lib/intel64_lin -lippcc -lippcore -lippi -L/usr/lib/x86_64-linux-gnu -lavcodec -lswscale -lavutil -lavformat -o unittest1.o

	g++ unittest1.o RGBToYCbCr_8u_P3R_test.o Copy_8u_C1R_test.o Copy_8u_C3P3R_test.o test_utils.o  -I/opt/intel/ipp/include -L/opt/intel/ipp/lib/intel64_lin -lippcc -lippcore -lippi -L/usr/lib/x86_64-linux-gnu -lavcodec -lswscale -lavutil -lavformat -o a
