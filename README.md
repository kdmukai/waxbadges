![achieveos](assets/achieveos_logo_1280.png)
# Achieveos
_Achieve(ments) + EOS = Achieveos_

An open platform for Achievements/Badges/Trophies that will be preserved forever on the EOS blockchain.

## Motivation
### Achievements should have permanence
Current achievement systems are completely siloed in their own ecosystems--XBox gamertag, individual mobile games, Steam trophies, even certifications for tech or skills training (e.g. Khan Academy badges). Because of this they all suffer from varying levels of impermanence and fragility. I can work hard to unlock a new badge in my running app ("50-Mile Club!"), but if that service shuts down, all of my badges go with it.

### Achievements should be accessible outside of their ecosystem
I shouldn't have to go to each individual ecosystem or sign into each individual app to see and share my achievements. But there's currently no way to view my accomplishments from Steam alongside all my mobile game achievements and everywhere else all in one place.

### Enter the blockchain
Blockchains are as close to immutable, permanent digital storage as we're going to get. Writing achievements to the blockchain will preserve them regardless of what happens to the Ecosystem or company that originally granted the achievement/trophy/badge. And once they're written to a public blockchain it's simple to view all of them in one grand trophy room, share any of them out to social media, etc. Permanence + unified public access.

## Achieveos overview
Achieveos is an EOS smart contract that provides a simple, open platform for any permanent achievement system to be built upon.

But because smart contracts are cumbersome to directly interact with, it's expected that richer web apps will be built on top of the Achieveos functionality. Think of Achieveos as a kind of services-rich backend database (AaaS? Achievements as a Service).

The first demonstration web app, eternalbadge.com (coming soon), will be an example of how the Achieveos smart contract can be used. The smart contract details will be totally hidden away from the users; they won't need to know anything about blockchains to be able to use the site. But crucially, should eternalbadge.com ever shut down, all of the achievements managed through the site will live on forever on the EOS blockchain.

It's the best of both worlds: A user-friendly UI but with permanently accessible, non-siloed data.

In fact, a future site could leverage the exact same data and provide a new UI. Users could pick up right where they left off (well, sort of) and carry on in this "new" world.


### Easy onboarding; "custodial" achievements
A big hurdle with blockchain-based user data systems is the overly complex onboarding process: would-be users have to convert fiat to crypto; set up access to those funds via tools like Metamask, Scatter, etc; and be comfortable signing transactions and managing their private/public keys. This is just not a reasonable expectation for 99% of gamers.

So instead Achieveos allows each project to add their users without worrying about whether or not the user has an on-chain account. Gamers' account records exist as simple `string` blockchain data based solely on the game's internal records (`name=Keith01`, `userid=your_internal_id_1234`). The studio can then immediately start granting achievements to their users. At this stage user achievements can be thought of as being held _in custody_ on their users' behalf.

These simple `User` entries within each studio's gaming ecosystem are totally isolated and siloed; a single person who plays multiple different games will have a separate `User` entry defined in each separate game with their associated achievements similarly siloed. This isn't ideal, but it's part of the compromise for having trivially simple onboarding within each game ecosystem.

However, there's a solution:


### Claim ownership; unify achievements
If a user has the interest and the savvy to create their own blockchain account, Achieveos provides a mechanism for them to claim their user identity in each studio's achievement ecosystem. This then allows them to view all of their achievements--across all their siloed gamer accounts--in one place.

In brief:
* Each studio would provide an option for a user to specify their blockchain account in their in-game profile.
* The studio would write this additional info to the gamer's `User` record on chain.
* The gamer can then submit a transaction to the Achieveos smart contract to "claim" each `User` entry and permanently tie them to their blockchain account.

After the claims are made it is then simple for a gamer to view all of their Achieveos achievements in one place via an Achieveos-aware block explorer.


### Structure
The basic organizational structure of Achieveos achievements is pretty simple:
```
    Ecosystem: "Banzai's Great Adventure"
    |
    +----Category: "Solo"
    |    |
    |    +----Achievement: "Coin Master"
    |    +----Achievement: "Treasure Finder"
    |    +----Achievement: "Grinder Extraordinaire"
    |
    |
    +----Category: "Team"
         |
         +----Achievement: "Purple Heart"
         +----Achievement: "My Savior"
         +----Achievement: "MVP"
         +----Achievement: "Da GOAT"
```

Each individual game would create its own **Ecosystem** entry. _Note that Achieveos doesn't have to be limited to just gaming use cases. An `Ecosystem` could be created for academic awards (e.g. a high school's NHS inductees), records for a sports team, certifications for a training system, etc._

A game studio creates a new `Ecosystem` in two simple steps:
* Create a blockchain account for their studio (or a separate account for each game they produce).
* Submit a simple transaction from that account to the Achieveos smart contract to create a new `Ecosystem` entry.

The Achieveos smart contract ensures that the studio's blockchain account is the only one that can then alter any of the data within that new `Ecosystem`.

They are then free to define whatever achievement **Categories** make sense for their game. _Note: at least one `Category` is required, but it can be a generic catch-all if the studio doesn't need different categories._

Finally they add various **Achievements** within a `Category`.

The actual `Achievement` entry consists of a title, description, and the name of an image asset (more on assets below):
```
{
    name: "Spicy Stunt Roll",
    description: "Rolled through fire while shielded",
    assetname: "spicy_roll.png"
}
```


### Assets
Images for each achievement are probably too much data to store on the blockchain. So instead each `Ecosystem` specifies an `assetbaseurl` (e.g. "mydomainname.com/images/trophies"). This is then combined with the `Achievement.assetname` to yield a complete url: https://mydomainname.com/images/trophies/spicy_roll.png.

The studio can always change the `Ecosystem.assetbaseurl` if they need to change domains, hosts, etc.

In this way we strike a compromise between providing nicely rendered achievement browsing without burdening game studios with excessive blockchain storage costs.


# Technical Notes

## Blockchain storage costs
The structure above was carefully designed to minimize blockchain storage costs. There are _**numerous**_ pitfalls when storing data to the blockchain that could prove _**very**_ costly if done poorly.

I learned this the hard way while developing the first version of this project for the EOS blockchain. I have a full writeup here: [RAM Rekt! EOS Storage Pitfalls](https://medium.com/@kdmukai_22159/ram-rekt-1eb8851b6fba). It is remarkable that a few minor design changes take the code from an impossibly cost-heavy _seems-great-in-theory-but-is-garbage-in-practice_ toy project to a truly viable, highly cost-effective achievements platform.


## EOS local dev
This project originally started on the EOS blockchain but has been migrated to WAX. However, because the WAX blockchain is a fork of `eosio` it fully supports EOS smart contracts. This means that we can continue to do local development against the well-tooled EOS blockchain, even if the WAX blockchain is our ultimate target.

### Install local EOS tools
We need to run a local dev EOS blockchain along with command line tools to interact with it. On a Mac:
```
brew tap eosio/eosio
brew install eosio
```

### WAX blockchain specifics
The WAX blockchain is currently only compatible with contracts compiled with an older version of the EOS Contract Development Toolkit: `eosio.cdt` v1.4.1

_Note that this is separate from the local blockchain we just installed above_

Install via Homebrew, targeting the v1.4.1 release's git hash:
```
brew unlink eosio.cdt
brew install https://raw.githubusercontent.com/EOSIO/homebrew-eosio.cdt/e6fc339b845219d8bc472b7a4ad0c146bd33752a/eosio.cdt.rb
```
_WAX also has their own v1.4.1 `eosio.cdt` release [here](https://github.com/worldwide-asset-exchange/wax-cdt)_


### Supported versions
Achieveos compiles with `eosio.cdt` v1.4.1.

Tests run successfully against the latest `eosio` node (currently v1.8.1).


## Running tests
Requirements:
* python3.6+
* virtualenv

The tests are written using [EOSFactory](https://eosfactory.io/) which makes it easy to write thorough and complex unit tests in Python.

Create a new python3 virtualenv.

Install the `eosfactory` python-based testing environment from Tokenika:
* Follow the installation instructions [here](https://github.com/tokenika/eosfactory/blob/master/docs/tutorials/01.InstallingEOSFactory.md). 

_Note: I had trouble getting things to work when I installed via PyPi, but the `git clone` option worked fine. YMMV._

_Note: If you're running in a `virtualenv` as I recommend, you'll need to edit the `install.sh` script and make the following change:_
```
# Original
pip3 install --user -e .

# Remove the '--user' flag for virtualenv compatibility
pip3 install -e .
```

EOSFactory will launch your local test node, reset the blockchain data to a clean state, generate user accounts, compile the smart contract, deploy it, and then execute the unit tests.

In theory the entire process is kicked off by a single command:
```
python test/test_achieveos.py
```

However, I ran into issues after stepping `eosio.cdt` down to v1.4.1. The automatic compilation step succeeded, but calls against the smart contract in the tests failed. But if we just keep the compilation step separate from running the EOSFactory tests, everything works just fine.

So I added two simple scripts:
* `compile.sh` compiles and then copies the resulting WASM and ABI files to EOSFactory's `build/` directory.
* `run_tests.py` runs the EOSFactory tests but disables the automatic compilation step.


## Running locally
If you want to directly interact with the smart contract on your local blockchain, there are quite a number of manual steps. But aside from being not super user-friendly, it's more or less straightforward:

Start kleos and nodeos:
```
keosd &
nodeos -e -p eosio \
  --plugin eosio::producer_plugin \
  --plugin eosio::chain_api_plugin \
  --plugin eosio::http_plugin \
  --plugin eosio::history_plugin \
  --plugin eosio::history_api_plugin \
  --access-control-allow-origin='*' \
  --contracts-console \
  --http-validate-host=false \
  --verbose-http-errors >> /dev/null 2>&1 &
```

Compile the smart contract:
```
eosio-cpp -o achieveos.wasm achieveos.cpp --abigen
```

Create initial dev wallet, save the password:
```
cleos wallet create --to-console

Creating wallet: default
Save password to use in the future to unlock this wallet.
Without password imported keys will not be retrievable.
"PW5Kewn9L76X8Fpd....................t42S9XCw2"
```

Open and unlock the wallet:
```
cleos wallet open
cleos wallet unlock
```

Create keys and copy public key:
```
cleos wallet create_key

> Created new private key with a public key of: "EOS8PEJ5FM42xLpHK...X6PymQu97KrGDJQY5Y"
```

Import the default dev 'eosio' key:
```
cleos wallet import

> private key: 5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3
```

Create test accounts:
```
cleos create account eosio bob EOSyourpublickeyfromabove
cleos create account eosio alice EOSyourpublickeyfromabove
```

Create the contract account:
```
cleos create account eosio achieveos EOSyourpublickeyfromabove -p eosio@active
```

Deploy the compiled contract:
```
cleos set contract achieveos /path/to/contracts/achieveos -p achieveos@active
```

Push some basic smart contract actions:
```
cleos push action achieveos addorg '["alice", "Awesome Ecosystem", "fakedomain.com/assets"]' -p alice@active
```

## Cleanup / Resetting
To stop kleos and nodeos:
```
pkill -SIGTERM nodeos
pkill -SIGTERM keosd
```

To reset the local chain's wallets:
```
rm -rf ~/eosio-wallet
```



# TODO / Future Features
* Simple Achieveos-aware block explorer to view achievements:
    * Browse by `Ecosystem`; see the possible `Achievements`, how many people were granted each `Achievement`
    * Browse by `User` in each `Ecosystem`; see which `Achievements` they were granted.
    * Browse by gamer's blockchain account; see their unified `Achievements` across all linked `Ecosystems`.
    * Social media sharing.

* Add support for a points system for each `Achievement`, point totals for `User`s?

* 
