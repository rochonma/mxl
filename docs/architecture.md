# Architecture

## Requirements

- Leverages the NMOS Flows and Grains model and resource schemas
- Lightweight SDK
  - Direct Memory Access, no additional memcpy, transfers, serialization, packetization, etc.
  - Miminal SDK. No large frameworks or large external libraries. Minimal dependencies.
  - No central point of failure
- Security
- Intra and Inter host
- Mixed CPU and GPU execution

## Shared memory model

## Security model

## Docker and Kubernetes

## API Usage Notes

### Video

- At the moment the only supported video media_type is "video/v210"
- The optional (but recommended) "grain_rate" attribute of the NMOS Flow resource is required for video and data flows in MXL.
- The grain_rate in NMOS expresses the frame rate. For interlaced video, users of the API must pass valid frame rates and not field rates. The only valid values for grain_rate for interlaced media is 25/1 or 30000/1001. Internally the SDK will convert it to a field rate (doubled rate)

### Audio

### Ancillary Data

Ancillary data payload is _exactly_ the SMPTE 2110-40 payload format without the RTP headers.
