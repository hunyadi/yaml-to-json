const strLookup = Object.fromEntries(
    Array.from('ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/').map((a, i) => [a, i])
);
strLookup['='] = 0;
function atob(base64) {
    base64 = base64.replace(/=/g, '');
    let n = base64.length;
    let rem = n % 4;
    let k = rem && rem - 1;  // how many bytes the last base64 chunk encodes
    let m = (n >> 2) * 3 + k;  // total encoded bytes
    base64 += '===';
    let bytes = new Uint8Array(m + 3);

    for (let i = 0, j = 0; i < n; i += 4, j += 3) {
        let x1 = strLookup[base64[i + 0]] << 18;
        let x2 = strLookup[base64[i + 1]] << 12;
        let x3 = strLookup[base64[i + 2]] << 6;
        let x4 = strLookup[base64[i + 3]];
        bytes[j + 0] = (x1 | x2) >> 16;
        bytes[j + 1] = ((x2 | x3) >> 8) & 0xff;
        bytes[j + 2] = (x3 | x4) & 0xff;
    }

    let arr = new Uint8Array(bytes.buffer, 0, m);
    return Array.from(arr).map((c) => String.fromCharCode(c)).join('');
}
