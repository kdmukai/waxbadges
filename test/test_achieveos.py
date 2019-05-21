import argparse
import sys
import unittest

from eosfactory.eosf import *


verbosity([Verbosity.INFO, Verbosity.OUT, Verbosity.TRACE, Verbosity.DEBUG])

CONTRACT_WORKSPACE = "achieveos"

# Actors of the test:
MASTER = MasterAccount()
HOST = Account()
ALICE = Account()
CAROL = Account()
BOB = Account()

skip_recompile = False

class Test(unittest.TestCase):

    def run(self, result=None):
        super().run(result)

    @classmethod
    def setUpClass(cls):
        SCENARIO("Reset local node, create test accounts, and run all tests")
        reset()
        create_master_account("MASTER")

        COMMENT("Create test accounts:")
        create_account("ALICE", MASTER)
        create_account("CAROL", MASTER)
        create_account("BOB", MASTER)


    def test_001_deploy(self):
        COMMENT("Create, build and deploy the contract:")
        create_account("HOST", MASTER)
        smart = Contract(HOST, CONTRACT_WORKSPACE)
        if not skip_recompile:
            smart.build()
        smart.deploy()


    def test_100_account_create_org(self):
        COMMENT("Should allow an EOS account to create an Organization")
        org_name = "Tournament of Champions"
        HOST.push_action(
            "addorg", {"org_owner": ALICE, "org_name": org_name},
            permission=(ALICE, Permission.ACTIVE),
            force_unique=True)

        table = HOST.table("orgs", ALICE)
        self.assertEqual(table.json["rows"][0]["org_name"], org_name)


    def test_100_contract_create_org(self):
        COMMENT("Should allow the contract owner to create an Organization on behalf of org_owner")
        org_name = "Some Other Alice Org"
        HOST.push_action(
            "addorg", {"org_owner": ALICE, "org_name": org_name},
            permission=(HOST, Permission.ACTIVE),
            force_unique=True)

        table = HOST.table("orgs", ALICE)
        self.assertEqual(table.json["rows"][1]["org_name"], org_name)


    def test_110_other_create_org(self):
        COMMENT("Should not allow a different account to create an Organization on behalf of org_owner")
        org_name = "Not Allowed Org"
        with self.assertRaises(Exception) as e:
            HOST.push_action(
                "addorg", {"org_owner": ALICE, "org_name": org_name},
                permission=(BOB, Permission.ACTIVE),
                force_unique=True)

        err_msg = str(e.exception)
        self.assertTrue("Not authorized" in err_msg, err_msg)


    def test_120_account_create_separate_org(self):
        COMMENT("Should allow a different EOS account to create its own isolated Organization")
        org_name = "A Totally Different Org"
        HOST.push_action(
            "addorg", {"org_owner": BOB, "org_name": org_name},
            permission=(BOB, Permission.ACTIVE),
            force_unique=True)

        # Because of the table scope, Bob's Org will also have index 0
        table = HOST.table("orgs", BOB)
        self.assertEqual(table.json["rows"][0]["org_name"], org_name)

        # While Alice still has her Orgs in her own table scope
        table = HOST.table("orgs", ALICE)
        self.assertTrue(table.json["rows"][0]["org_name"] != org_name)


    @classmethod
    def tearDownClass(cls):
        stop()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="""
        This is a unit test for the achieveos smart contract.
    """)

    parser.add_argument(
        "-s", "--skip_recompile",
        action="store_true",
        help="Don't recompile the contract")

    args = parser.parse_args()

    skip_recompile = args.skip_recompile

    unittest.main(argv=[sys.argv[0]])
