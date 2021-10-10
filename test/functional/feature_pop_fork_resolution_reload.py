#!/usr/bin/env python3
# Copyright (c) 2014-2019 The Bitcoin Core developers
# Copyright (c) 2019-2021 Xenios SEZC
# https://www.veriblock.org
# Distributed under the MIT software license, see the accompanying
# file LICENSE or http://www.opensource.org/licenses/mit-license.php.

from test_framework.pop import endorse_block, create_endorsed_chain, mine_until_pop_active, get_keystone_interval
from test_framework.test_framework import BitcoinTestFramework
from test_framework.util import (
    connect_nodes,
    disconnect_nodes, assert_equal,
)

class PopFr(BitcoinTestFramework):
    def set_test_params(self):
        self.setup_clean_chain = True
        self.num_nodes = 6
        self.extra_args = [["-txindex"], ["-txindex"], ["-txindex"], ["-txindex"], ["-txindex"], ["-txindex"]]
        self.extra_args = [x + ['-debug=cmpctblock'] for x in self.extra_args]
        self.extra_args = [x + ['-poplogverbosity=info'] for x in self.extra_args]

    def skip_test_if_missing_module(self):
        self.skip_if_no_wallet()
        self.skip_if_no_pypoptools()

    def setup_network(self):
        self.setup_nodes()
        mine_until_pop_active(self.nodes[0])

        # all nodes connected and synced
        for i in range(self.num_nodes - 1):
            connect_nodes(self.nodes[i + 1], i)
        self.sync_all()
        self.keystoneInterval = get_keystone_interval(self.nodes[0])

    def get_best_block(self, node):
        hash = node.getbestblockhash()
        return node.getblock(hash)

    def _reload_test(self):
        self.log.info("starting _reload_test()")

        self.log.info("generate orphan blocks")
        for a in range(0, 100):
            self.log.info("mine block: {}".format(a))
            for i in range(self.num_nodes):
                for j in range(self.num_nodes):
                    if i != j:
                        disconnect_nodes(self.nodes[i], j)
        
            self.nodes[0].generate(nblocks=2)
            self.nodes[1].generate(nblocks=1)
            self.nodes[2].generate(nblocks=1)
            self.nodes[3].generate(nblocks=1)
            self.nodes[4].generate(nblocks=1)
            self.nodes[5].generate(nblocks=1)

            # all nodes connected and synced
            for i in range(self.num_nodes - 1):
                connect_nodes(self.nodes[i + 1], i)
            
            self.sync_blocks(self.nodes, timeout=30)

            
        # 110 blocks without endorsements
        self.nodes[0].generate(nblocks=110)
        tip = self.get_best_block(self.nodes[0])
        
        self.sync_blocks(self.nodes, timeout=30)
        self.log.info("nodes[0,1,2,3,4,5] are in sync")

        assert tip == self.get_best_block(self.nodes[0]) and tip == self.get_best_block(self.nodes[1]) and tip == self.get_best_block(self.nodes[2]) and tip == self.get_best_block(self.nodes[3]) and tip == self.get_best_block(self.nodes[4]) and tip == self.get_best_block(self.nodes[5])

        self.log.info("restart node 0")
        self.stop_node(0)
        self.start_node(0)

        self.log.info("connect all nodes")
        # all nodes connected and synced
        for i in range(self.num_nodes - 1):
            connect_nodes(self.nodes[i + 1], i)

        self.nodes[1].generate(nblocks=100)

        assert False

        self.log.warning("_reload_test() succeeded!")

    def run_test(self):
        """Main test logic"""

        self.sync_all(self.nodes[0:4])

        from pypoptools.pypopminer import MockMiner
        self.apm = MockMiner()

        self._reload_test()


if __name__ == '__main__':
    PopFr().main()