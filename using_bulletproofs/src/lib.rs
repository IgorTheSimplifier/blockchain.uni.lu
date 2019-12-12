 // The #-commented lines are hidden in Rustdoc but not in raw
 // markdown rendering, and contain boilerplate code so that the
 // code in the README.md is actually run as part of the test suite.

extern crate rand;
use rand::thread_rng;

extern crate curve25519_dalek;
use curve25519_dalek::scalar::Scalar;
use curve25519_dalek::ristretto::CompressedRistretto;

extern crate merlin;
use merlin::Transcript;

extern crate bulletproofs;
use bulletproofs::{BulletproofGens, PedersenGens, RangeProof};
use std::ffi::CString;

extern crate libc;
use libc::c_char;
use std::ffi::CStr;

extern crate hex;
/////////////////////////////
fn create_age_bulletproof(age: u64) -> (String, String) {
	// println!("Age is: {}", age);

	// pc_gens::Generators for Pedersen commitments.  These can be selected
	// independently of the Bulletproofs generators.
	// bp_gens::Generators for Bulletproofs, valid for proofs up to bitsize "bits"
	// and aggregation size up to 1.
	let bits = 8;
	let pc_gens = PedersenGens::default();
	let bp_gens = BulletproofGens::new(bits, 1);
	
	let minAge = 18;
	let secret_value = ((age as i64) - minAge) as u64;
	let blinding = Scalar::random(&mut thread_rng());

	// The proof can be chained to an existing transcript.
	// Here we create a transcript with a doctest domain separator.
	let mut prover_transcript = Transcript::new(b"doctest example");

	// Create a 32-bit rangeproof.
	let (proof, committed_value) = RangeProof::prove_single(
		&bp_gens,
		&pc_gens,
		&mut prover_transcript,
		secret_value,
		&blinding,
		bits,
	).expect("A real program could handle errors");

	return (hex::encode(proof.to_bytes()), hex::encode(committed_value.to_bytes()));
}


fn verify_age_bulletproof(proof: String, committed_value: String) -> bool {
	// pc_gens::Generators for Pedersen commitments.  These can be selected
	// independently of the Bulletproofs generators.
	// bp_gens::Generators for Bulletproofs, valid for proofs up to bitsize "bits"
	// and aggregation size up to 1.
	let bits = 8;
	let pc_gens = PedersenGens::default();
	let bp_gens = BulletproofGens::new(bits, 1);


	let deserealizedProofResult = RangeProof::from_bytes(hex::decode(proof).unwrap().as_slice());

	match deserealizedProofResult {
		Ok(v) => {
			let deserealizedProof = v;
			
			let mut verifier_transcript = Transcript::new(b"doctest example");
			let deserealizedCommittedValue = CompressedRistretto::from_slice(hex::decode(committed_value).unwrap().as_slice());

			return deserealizedProof
							.verify_single(&bp_gens
											, &pc_gens
											, &mut verifier_transcript
											, &deserealizedCommittedValue
											, bits)
							.is_ok()
	},
		Err(e) => {
			return false;
		}
	}
}


#[no_mangle]
pub extern fn create_encoded_age_bulletproof(age: u64) -> *const c_char {
	let (proof, cv) = create_age_bulletproof(age);
	let encoded =  format!("{}{}", proof, cv);
	let s = CString::new(encoded).unwrap();
	let p = s.as_ptr();
	std::mem::forget(s);
	return p;
}

#[no_mangle]
pub extern fn verify_encoded_age_bulletproof(encoded: *const c_char) -> bool {
	let encoded: &CStr = unsafe { CStr::from_ptr(encoded) };
	let encoded: String = encoded.to_str().unwrap().to_owned();

	if (encoded.len() != 1024) {
		return false;
	}

	let mut proof = encoded;
	let cv = proof.split_off(proof.len()-64);

	if(cv.len() != 64) {
		return false;
	}
	
	return verify_age_bulletproof(proof, cv);
}