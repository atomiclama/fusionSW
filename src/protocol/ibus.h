

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

decodeRes_e iBusDecode(radioPacket_t* out, radioPacket_t* in);
decodeRes_e iBusEncodeAir(radioPacket_t* out, radioPacket_t* in);



#ifdef __cplusplus
}
#endif
