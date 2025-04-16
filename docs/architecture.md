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


### Grain I/O

Grain I/O can be 'partial'. In other words, a FlowWriter can write the bytes of a grain progressively (as slices for example).  

The following example demonstrates how a FlowWriter may write a grain as slices using multiple ```mxlFlowWriterCommit()``` calls.  The important field here is the GrainInfo.commitedSize: it tells us how many bytes in the grain are valid.  A complete grain will have ```GrainInfo.commitedSize == GrainIinfo.grainSize```.  It is imperative that FlowReaders _always_ take into consideration the ```GrainInfo.commitedSize``` field before making use of the grain buffer.  The ```mxlFlowReaderGetGrain``` function will return as soon as new data is commited to the grain. 

### Example Grain I/O (from the [unit tests](../lib/tests/test_flows.cpp))

```c++ 
    ///
    /// Write and read a grain in 8 slices
    ///

    /// Open the grain.
    GrainInfo gInfo;
    uint8_t *buffer = nullptr;
    
    /// Open the grain for writing.
    REQUIRE( mxlFlowWriterOpenGrain( instanceWriter, writer, index, &gInfo, &buffer ) == MXL_STATUS_OK );

    const size_t maxSlice = 8;
    auto sliceSize = gInfo.grainSize / maxSlice;
    for ( size_t slice = 0; slice < maxSlice; slice++ )
    {
        /// Write a slice the grain.
        gInfo.commitedSize += sliceSize;
        REQUIRE( mxlFlowWriterCommit( instanceWriter, writer, &gInfo ) == MXL_STATUS_OK );

        FlowInfo sliceFlowInfo;
        REQUIRE( mxlFlowReaderGetInfo( instanceReader, reader, &sliceFlowInfo ) == MXL_STATUS_OK );
        REQUIRE( sliceFlowInfo.headIndex == index );
     
        /// Read back the partial grain using the flow reader.
        REQUIRE( mxlFlowReaderGetGrain( instanceReader, reader, index, 8, &gInfo, &buffer ) == MXL_STATUS_OK );

        // Validate the commited size
        REQUIRE( gInfo.commitedSize == sliceSize * ( slice + 1 ) );
    }
```

### Video

- At the moment the only supported video media_type is "video/v210"
- The optional IS-04 (but recommended) "grain_rate" attribute of the NMOS Flow resource is mandatory for video and data flows in MXL.
- The grain_rate in NMOS expresses the frame rate. For interlaced video, users of the API must pass valid frame rates and not field rates. The only valid values for grain_rate for interlaced media is 25/1 or 30000/1001. Internally the SDK will convert it to a field rate (doubled rate)

### Audio

### Ancillary Data

Ancillary data payload is _exactly_ the SMPTE 2110-40 payload format without the RTP headers.
