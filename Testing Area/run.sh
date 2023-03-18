runnableName=test_main.exe
fileName=test_ssl.cpp
g++ -I. -o ${runnableName} ${fileName} -lcrypto -lssl
./${runnableName}