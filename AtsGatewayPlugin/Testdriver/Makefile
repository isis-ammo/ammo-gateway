
SRC_DIR:=.
DST_DIR:=.

run: build 
	python AndroidGateway_Ats.py localhost 32869 pull	

build: ${SRC_DIR}/DataMessage.proto
	protoc -I=${SRC_DIR} --python_out=${DST_DIR} ${SRC_DIR}/DataMessage.proto
	cd ..; make

abuse: build 
	while true ; do
	python AndroidGateway_Ats.py localhost 32869 subscribe &
	sleep 3;
	kill %+
	done
