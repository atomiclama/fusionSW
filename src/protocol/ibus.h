

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

decodeRes_e iBusDecode(radioPacket_t* in, radioPacket_t* out);
decodeRes_e iBusEncode(radioPacket_t* in, radioPacket_t* out);



#ifdef __cplusplus
}
#endif
