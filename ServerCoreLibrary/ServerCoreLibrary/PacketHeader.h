#pragma once
struct PacketHeader {
  int size;
  int groupId;
  int id;  // 프로토콜ID (ex. 1=로그인, 2=이동요청)
};
