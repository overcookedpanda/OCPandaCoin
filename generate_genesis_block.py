import argparse
import csv
from pathlib import Path

import multiprocessing as mp
from btcutil import *
import time
import asyncio

def read_balances(file: Path):
    assert file.exists()
    with open(file, newline='') as csvfile:
        reader = csv.reader(csvfile, delimiter=',')
        for script, satoshis, _ in reader:
            try:
                bytes.fromhex(script)
            except:
                raise Exception("Looks like {} is not a script".format(script))
            yield CTxOut(CScript() + script, satoshis)


def generate_genesis_block(
        nTime: int,
        nBits: int,
        nVersion: int,
        pszTimestamp: str = "Overcooked Panda",
        txouts: List[CTxOut] = []
):
    PREV = b'\x00' * 32

    # create input
    scriptSig = CScript() + script_with_prefix(nBits) + pszTimestamp.encode('ascii')

    # create coinbase tx
    tx = Tx(
        version=nVersion,
        inputs=[CTxIn(scriptSig=scriptSig)],
        outputs=txouts
    )
    merkleroot = tx.tx_id()
    TIME = nTime
    while True:
        nonce = 0
        header = make_header(
            nVersion,
            PREV,
            merkleroot,
            TIME,
            nBits,
            nonce
        )

        before = time.time()
        target = decode_target_int(nBits)
        while get_block_hash_int(header) >= target:
            nonce += 1
            if nonce > 0xffffffff:
                nonce = 0
                TIME += 1
                print("Increasing timestamp {}".format(TIME))
                set_header_timestamp(header, TIME)
            set_header_nonce(header, nonce)
        after = time.time()
        if get_block_hash_int(header) < target:
            print(f'''Found: 
                Nonce:      {nonce}
                Header:     {header.hex()}
                Hash:       {get_block_hash(header).hex()}
                MerkleRoot: {merkleroot[::-1].hex()}
                Tx:         {tx.encode().hex()} 
                Took:       {after-before}
            ''')
            return
        else:
            print("Can not find a solution...")

def main():
    pszTimestamp = "Overcooked Panda forks VeriBlock reference implementation for PoP enabled coins - Feb 1, 2022"
    timestamp = 1643754621

    out = CTxOut(
        scriptPubKey=CScript() + OpCode(0x00) + bytes.fromhex("3dd5f2f667315cc98e669deb88d3dfe831aa2cea"),
        amount=50 * 10**8
    )

    print("Regtest")
    generate_genesis_block(
        nTime=timestamp,
        pszTimestamp=pszTimestamp,
        nBits=0x207fffff,
        nVersion=1,
        txouts=[out]
    )

    print("Testnet")
    generate_genesis_block(
        nTime=timestamp,
        pszTimestamp=pszTimestamp,
        nBits=0x1d07ffff,
        nVersion=1,
        txouts=[out]
    )

    print("Mainnet")
    generate_genesis_block(
        nTime=timestamp,
        pszTimestamp=pszTimestamp,
        nBits=0x1d00ffff,
        nVersion=1,
        txouts=[out]
    )


if __name__ == "__main__":
    main()
