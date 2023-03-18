runnableName=test_main
fileName=test_ssl.cpp
g++ -o ${runnableName} ${fileName} -lcrypto -lssl
./${runnableName}