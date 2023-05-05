use novelpoly::{CodeParams, WrappedShard};
use std::os::raw::c_ulong;
use std::slice;

const MAX_VALIDATORS: usize = novelpoly::f2e16::FIELD_SIZE;

/// Errors in erasure coding.
#[repr(C)]
#[derive(Debug, Clone, PartialEq)]
pub enum NPRSResult {
    /// No error
    Ok,
    /// Returned when there are too many validators.
    TooManyValidators,
    /// Cannot encode something for zero or one validator
    NotEnoughValidators,
    /// Cannot reconstruct: wrong number of validators.
    WrongValidatorCount,
    /// Not enough chunks present.
    NotEnoughChunks,
    /// Too many chunks present.
    TooManyChunks,
    /// Chunks not of uniform length or the chunks are empty.
    NonUniformChunks,
    /// An uneven byte-length of a shard is not valid for `GF(2^16)` encoding.
    UnevenLength,
    /// Chunk index out of bounds.
    ChunkIndexOutOfBounds {
        /// index of invalid chunk
        chunk_index: c_ulong,
        /// number of validators
        n_validators: c_ulong,
    },
    /// Bad payload in reconstructed bytes.
    BadPayload,
    /// Invalid branch proof.
    InvalidBranchProof,
    /// Branch out of bounds.
    BranchOutOfBounds,
    /// Unknown error
    UnknownReconstruction,
    /// Unknown error
    UnknownCodeParam,
}

/// Represent the data array
#[repr(C)]
pub struct DataBlock {
    array: *mut u8,
    length: c_ulong,
}

/// Represent chunk of the data
#[repr(C)]
pub struct Chunk {
    data: DataBlock,
    index: c_ulong,
}

/// Represent the array of chunks
#[repr(C)]
pub struct ChunksList {
    data: *mut Chunk,
    count: c_ulong,
}

/// Obtain a threshold of chunks that should be enough to recover the data.
pub const fn recovery_threshold(n_validators: usize) -> Result<usize, NPRSResult> {
    if n_validators > MAX_VALIDATORS {
        return Err(NPRSResult::TooManyValidators);
    }

    if n_validators <= 1 {
        return Err(NPRSResult::NotEnoughValidators);
    }

    let needed = n_validators.saturating_sub(1) / 3;
    Ok(needed + 1)
}

fn code_params(n_validators: usize) -> Result<CodeParams, NPRSResult> {
    // we need to be able to reconstruct from 1/3 - eps

    let n_wanted = n_validators;
    let k_wanted = recovery_threshold(n_wanted)?;

    if n_wanted > MAX_VALIDATORS as usize {
        return Err(NPRSResult::TooManyValidators);
    }

    CodeParams::derive_parameters(n_wanted, k_wanted).map_err(|e| match e {
        novelpoly::Error::WantedShardCountTooHigh(_) => NPRSResult::TooManyValidators,
        novelpoly::Error::WantedShardCountTooLow(_) => NPRSResult::NotEnoughValidators,
        _ => NPRSResult::UnknownCodeParam,
    })
}

/// Obtain a threshold of chunks that should be enough to recover the data.
/// @param n_validators determines the number of validators to shard data for
/// @param threshold_out output recovery threshold value
///
#[allow(unused_attributes)]
#[no_mangle]
pub unsafe extern "C" fn ECCR_get_recovery_threshold(
    validators_number: c_ulong,
    threshold_out: *mut c_ulong,
) -> NPRSResult {
    debug_assert!(!threshold_out.is_null());

    let n_validators = validators_number as usize;
    match recovery_threshold(n_validators) {
        Ok(needed) => {
            *threshold_out = needed as c_ulong;
            return NPRSResult::Ok;
        }
        Err(e) => return e,
    }
}

/// Cleans the data block
#[allow(unused_attributes)]
#[no_mangle]
pub unsafe extern "C" fn ECCR_deallocate_data_block(data: *mut DataBlock) {
    debug_assert!(!data.is_null());
    debug_assert!(!(*data).array.is_null());
    drop(Box::from_raw((*data).array));
}

/// Cleans the data in chunk
#[allow(unused_attributes)]
#[no_mangle]
pub unsafe extern "C" fn ECCR_deallocate_chunk(data: *mut Chunk) {
    debug_assert!(!data.is_null());
    ECCR_deallocate_data_block(&mut (*data).data);
}

/// Cleans the data allocated for the chunk list
#[allow(unused_attributes)]
#[no_mangle]
pub unsafe extern "C" fn ECCR_deallocate_chunk_list(chunk_list: *mut ChunksList) {
    debug_assert!(!chunk_list.is_null());
    debug_assert!(!(*chunk_list).data.is_null());
    let data = std::mem::transmute::<*mut ChunksList, &mut ChunksList>(chunk_list);
    {
        let chunks = slice::from_raw_parts_mut(data.data, data.count as usize);
        for chunk in chunks.iter_mut() {
            ECCR_deallocate_chunk(chunk);
        }
    }
    drop(Box::from_raw(data.data));
}

/// Obtain erasure-coded chunks, one for each validator.
///
/// Works only up to 65536 validators, and `n_validators` must be non-zero.
#[allow(unused_attributes)]
#[no_mangle]
pub unsafe extern "C" fn ECCR_obtain_chunks(
    validators_number: c_ulong,
    message: *const DataBlock,
    output: *mut ChunksList,
) -> NPRSResult {
    debug_assert!(!message.is_null());
    debug_assert!(!output.is_null());
    debug_assert!(!(*message).array.is_null());
    debug_assert!(!(*message).length > 0);

    let n_validators = validators_number as usize;
    let encoded = slice::from_raw_parts((*message).array, (*message).length as usize);
    let params = match code_params(n_validators) {
        Ok(p) => p,
        Err(e) => return e,
    };

    let shards = params
        .make_encoder()
        .encode::<WrappedShard>(&encoded[..])
        .expect("Payload non-empty, shard sizes are uniform, and validator numbers checked; qed");

    let mut output_chunks = vec![];
    for (index, name) in shards.into_iter().enumerate() {
        let mut v = name.into_inner();
        let db = DataBlock {
            array: v.as_mut_ptr(),
            length: v.len() as _,
        };
        let chunk = Chunk {
            data: db,
            index: index as _,
        };
        std::mem::forget(v);
        output_chunks.push(chunk);
    }

    let o = std::mem::transmute::<*mut ChunksList, &mut ChunksList>(output);
    o.data = output_chunks.as_mut_ptr();
    o.count = output_chunks.len() as _;
    std::mem::forget(output_chunks);

    NPRSResult::Ok
}

/// Reconstruct data from a set of chunks.
///
/// Provide an iterator containing chunk data and the corresponding index.
/// The indices of the present chunks must be indicated. If too few chunks
/// are provided, recovery is not possible.
///
/// Works only up to 65536 validators, and `n_validators` must be non-zero.
#[allow(unused_attributes)]
#[no_mangle]
pub unsafe extern "C" fn ECCR_reconstruct(
    validators_number: c_ulong,
    input_chunks: *const ChunksList,
    outdata: *mut DataBlock,
) -> NPRSResult {
    debug_assert!(!outdata.is_null());
    debug_assert!(!input_chunks.is_null());
    debug_assert!(!(*input_chunks).data.is_null());
    debug_assert!(!(*input_chunks).count > 0);

    let n_validators = validators_number as usize;
    let params = match code_params(n_validators) {
        Ok(p) => p,
        Err(e) => return e,
    };
    let mut received_shards: Vec<Option<WrappedShard>> = vec![None; n_validators];
    let mut shard_len = None;
    let chunks = slice::from_raw_parts((*input_chunks).data, (*input_chunks).count as usize);

    for chunk in chunks.into_iter().take(n_validators) {
        if chunk.index as usize >= n_validators {
            return NPRSResult::ChunkIndexOutOfBounds {
                chunk_index: chunk.index,
                n_validators: n_validators as u64,
            };
        }

        if chunk.data.array.is_null() || chunk.data.length == 0 {
            continue;
        }

        let shard_len = shard_len.get_or_insert_with(|| chunk.data.length);
        if *shard_len % 2 != 0 {
            return NPRSResult::UnevenLength;
        }

        if *shard_len != chunk.data.length || *shard_len == 0 {
            return NPRSResult::NonUniformChunks;
        }

        received_shards[chunk.index as usize] = Some(WrappedShard::new(
            slice::from_raw_parts(chunk.data.array, chunk.data.length as usize).to_vec(),
        ));
    }

    let mut payload_bytes = match params.make_encoder().reconstruct(received_shards) {
        Err(e) => match e {
            novelpoly::Error::NeedMoreShards { .. } => return NPRSResult::NotEnoughChunks,
            novelpoly::Error::ParamterMustBePowerOf2 { .. } => return NPRSResult::UnevenLength,
            novelpoly::Error::WantedShardCountTooHigh(_) => return NPRSResult::TooManyValidators,
            novelpoly::Error::WantedShardCountTooLow(_) => return NPRSResult::NotEnoughValidators,
            novelpoly::Error::PayloadSizeIsZero { .. } => return NPRSResult::BadPayload,
            novelpoly::Error::InconsistentShardLengths { .. } => {
                return NPRSResult::NonUniformChunks
            }
            _ => return NPRSResult::UnknownReconstruction,
        },
        Ok(payload_bytes) => payload_bytes,
    };

    (*outdata).array = payload_bytes.as_mut_ptr();
    (*outdata).length = payload_bytes.len() as _;
    std::mem::forget(payload_bytes);

    NPRSResult::Ok
}
