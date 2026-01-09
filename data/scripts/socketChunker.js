function socketChunker(userHandler, port){
	if (typeof userHandler != "function"){
		this.error = "socketChunker - first argument must be handler";
		return;
		}
	if (typeof port != "number"){ 
		this.error = "socketChunker - second argument must be port number";
		return;
		}
	this.error = false;
	// declare propeties not to be included in enumeration
	Object.defineProperty(this, "trace", {enumerable: false, writable: true});
	Object.defineProperty(this, "_msgId", {enumerable: false, writable: true});
	this.trace = false;
	this._msgId = 0;

// ---- helpers ----

	function abByteLen(ab) {
		// Duktape: CBOR.encode returns ArrayBuffer
		return ab.byteLength;
		}

	function abSliceCopy(ab, start, end) {
		// Returns a *tight* ArrayBuffer copy [start, end)
		var src = new Uint8Array(ab);
		var n = end - start;
		var out = new Uint8Array(n);
		out.set(src.subarray(start, end));
		return out.buffer;
		}

	function fnv1a32_hex(bufAb) {
		// FNV-1a over bytes in an ArrayBuffer; returns 8-hex string
		var v = new Uint8Array(bufAb);
		var h = 0x811c9dc5; // offset basis
		for (var i = 0; i < v.length; i++) {
			h ^= v[i];
			h = (h * 0x01000193) >>> 0;
			}
		var s = h.toString(16);
		return ("00000000".slice(s.length) + s);
		}

	function makeChunk(id, seq, total, dataAb, hashHex) {
		// keep keys short
		return { c: 1, 	// 
			i: id,		// message id
			s: seq,		// seuence number of this chunk
			t: total,		// total number of chunks
			d: dataAb,	// the CBOR encoded object
			h: hashHex };	// FNV check hash
		}

	function encodedChunkSizeBytes(id, seq, total, dataAb) {
		// hash placeholder so size measurement is stable
		var obj = makeChunk(id, seq, total, dataAb, "00000000");
		var ab = CBOR.encode(obj);
		return abByteLen(ab);
		}

	function concatArrayBuffers(arr) {
		var total = 0;
		for (var i = 0; i < arr.length; i++) {
		total += arr[i].byteLength;
			}
		var out = new Uint8Array(total);
		var pos = 0;
		for (var i = 0; i < arr.length; i++) {
			out.set(new Uint8Array(arr[i]), pos);
			pos += arr[i].byteLength;
			}
		return out.buffer;
		}

	// end of helpers

	this._deliver = function(error, object){	// deliver to user
		this.userHandler(error, object);
		}

	this.socketSend = function(value, port, address) {
		const MAX_BYTES = 512;	// max UDP payload you want for *each datagram*

		// 1) Encode the whole value once to CBOR bytes
		var payloadAb = CBOR.encode(value);
		var payloadLen = abByteLen(payloadAb);
		var id = this._msgId++;
		var chunks = [];
		var pos = 0;
		var seq = 0;
		if (this.trace) printOrange("Ready to start chunking\n");
		while (pos < payloadLen) {
			// Find largest [pos,end) that fits after CBOR-encoding the *chunk envelope*
			var lo = pos + 1;
			var hi = payloadLen;
			var best = lo;

			// Binary search is faster than repeated stringify/encode loops
			while (lo <= hi) {
				var mid = (lo + hi) >> 1;
				var sliceAb = abSliceCopy(payloadAb, pos, mid);
				var size = encodedChunkSizeBytes(id, seq, 0, sliceAb);
				if (size <= MAX_BYTES) {
					best = mid;
					lo = mid + 1;
					}
				else hi = mid - 1;
				}

			if (best === pos)
				throw("Chunk envelope overhead exceeds MAX_BYTES=" + MAX_BYTES);
			var dataAb = abSliceCopy(payloadAb, pos, best); // Commit the best slice

			// 2) Build chunk with placeholder hash, encode, then compute FNV, then encode again
			var chunkObj = makeChunk(id, seq, 0, dataAb, "00000000");
			var tmpAb = CBOR.encode(chunkObj);
			chunkObj.h = fnv1a32_hex(tmpAb);

			// 3) Final encoded chunk to send
			var chunkAb = CBOR.encode(chunkObj);
			if (abByteLen(chunkAb) > MAX_BYTES) {
				 // Very rare, but safe guard (hash didn’t change length, but keep it anyway)
				 throw new Error("Internal: chunk exceeds MAX_BYTES after hash");
				}

			chunks.push(chunkAb);
			pos = best;
			seq++;
			}

		// Patch totals and resend-encode each chunk (total affects bytes => affects hash)
		var total = chunks.length;
		for (var k = 0; k < total; k++) {
			var chunk = chunks[k];
			var obj = CBOR.decode(chunk); // decode to patch 't'
			obj.t = total;
			obj.h = "00000000";
			var tmpAb2 = CBOR.encode(obj);
			obj.h = fnv1a32_hex(tmpAb2);
			chunk = CBOR.encode(obj);
			switch (arguments.length){
				case 1: socketSend(this.socketId, chunk); break;
				case 2: socketSend(this.socketId, chunk, port); break;
				case 3: socketSend(this.socketId, chunk, port, address); break;
				default: throw("socketChunker socketSend invalid call");
				}
			}
		}

	this.userHandler = userHandler;

	this._onChunk = function (error, bufAb) {
		if (this.trace) printOrange("Local handler received chunk\n");
		if (error){
			this._deliver(error, "socketChunker receive got error " + error + " from socker");
			return;
			}
		if (!this._rx) this._rx = Object.create(null); // first time in
		if (!this._completed) this._completed = Object.create(null);	// for completed
		var chunk;
		try {	// ---- decode CBOR chunk ----
			chunk = CBOR.decode(bufAb);
			} catch (e) {
			this._deliver(10, "socketChunker receive CBOR decode error");
			return;
			}
		if (this.trace) printOrange("decoded chunk OK\n");
		// Expected envelope:
		// { c:1, i:id, s:seq, t:total, d:ArrayBuffer, h:"xxxxxxxx" }

		if (!chunk || chunk.c !== 1) {
			this._deliver(11, "socketChunker receive invalid chunk envelope");
			return;
		}

		// ---- verify FNV ----
		var recvHash = chunk.h;
		chunk.h = "00000000";
		var checkAb = CBOR.encode(chunk);
		if (fnv1a32_hex(checkAb) !== recvHash) {
			this._deliver(12, "socketChunker receive fnv hash check failed");
			return;
			}
		if (this.trace) printOrange("fnv1a check OK\n");

		// ---- indices ----
		var id = chunk.i;
		if (this._completed[id]) return;	// ignore duplicates of completed messages
		var seq = chunk.s;
		var total = chunk.t;
		if (this.trace)printOrange("indices id:", id, " seq:", seq, " total:", total, "\n");

		if (total <= 0 || seq < 0 || seq >= total) {
			this._deliver(13, "socketChunker receive bad indicies");
			}

		// ---- message state ----
		var msg = this._rx[id];
		if (!msg) {
			msg = {
				total: total,
				received: 0,
				chunks: new Array(total)
				};
			this._rx[id] = msg;
			}
		if (this.trace) printOrange("message state OK\n");

		if (msg.total !== total) {
			delete this._rx[id];
			this._deliver(14, "socketChunker receive inconsistent total");
			return;
			}

		// ---- store chunk (ignore duplicates) ----
		if (!msg.chunks[seq]) {
			msg.chunks[seq] = chunk.d;
			msg.received++;
			}
		if (this.trace) printOrange("stored chunk OK\n");


		// ---- complete? ----
		if (msg.received === msg.total) {
			delete this._rx[id];
			try {
				var payloadAb = concatArrayBuffers(msg.chunks);
				var object = CBOR.decode(payloadAb);
				}
			catch (e) {
				this._deliver(15, "socketChunker receive reassembly / decode error");
				}
			this._completed	[id] = Date.now();	// make record of completion
			this._deliver(0, object);
			_cleanupExpired();	// clean up any expired records
			}
		}.bind(this);

	_cleanupExpired = function () {
		if (!this._completed) {
		   return;
			}
		var now = Date.now();
		var cutoff = now - 5000;   // 5 seconds
		var id;
		for (id in this._completed) {
		   if (this._completed[id] < cutoff) {
			  delete this._completed[id];
			   }
		    }
		};
		
	this.socketId = onSocketEvent(this._onChunk, port);
	}

