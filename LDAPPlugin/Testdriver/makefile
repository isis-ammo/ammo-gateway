
PB_DIR:=./../../common/protocol
SRC_DIR:=.
DST_DIR:=.

default: build


build: ${PB_DIR}/AmmoMessages.proto
	protoc --proto_path=${SRC_DIR} --proto_path=${PB_DIR} --python_out=${DST_DIR} ${PB_DIR}/AmmoMessages.proto
	#cd ..; make

