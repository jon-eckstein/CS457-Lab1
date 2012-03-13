
#pragma pack(1)
struct replyMessage{
    uint8_t version;
};
#pragma pack(0)

#pragma pack(1)
struct clientMessage {
    uint8_t version;
    int number;
};
#pragma pack(0)

