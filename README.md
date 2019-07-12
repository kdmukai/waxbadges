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
Blockchains are as close to immutable, permanent digital storage as we're going to get. Writing achievements to the blockchain will preserve them regardless of what happens to the organization or company that originally granted the achievement/trophy/badge. And once they're written to a public blockchain it's simple to view all of them in one grand trophy room, share any of them out to social media, etc. Permanence + unified public access.

## Achieveos overview
Achieveos is an EOS smart contract that provides a simple, open platform for any permanent achievement system to be built upon.

But because smart contracts are cumbersome to directly interact with, it's expected that richer web apps will be built on top of the Achieveos functionality. Think of Achieveos as a kind of services-rich backend database (AaaS? Achievements as a Service).

The first demonstration web app, eternalbadge.com (coming soon), will be an example of how the Achieveos smart contract can be used. The smart contract details will be totally hidden away from the users; they won't need to know anything about blockchains to be able to use the site. But crucially, should eternalbadge.com ever shut down, all of the achievements managed through the site will live on forever on the EOS blockchain.

It's the best of both worlds: A user-friendly UI but with permanently accessible, non-siloed data.

In fact, a future site could leverage the exact same data and provide a new UI. Users could pick up right where they left off (well, sort of) and carry on in this "new" world.

### Easy onboarding; "custodial" achievements
A big hurdle with blockchain-based users is the overly complex onboarding process: converting fiat to crypto; setting up access to those funds via tools like Metamask, Scatter, etc; generating and signing transactions to interact with a smart contract. This is just not a reasonable expectation yet for the vast majority of users.

So instead each project can add their own users as simple `string` blockchain data (`name=Keith01`, `userid=your_internal_id_1234`) and immediately start granting achievements to that user. The achievements are written to the blockchain but in this case the user has not yet asserted any control over their gamer identity nor claimed ownership of their achievements. In a sense their achievements are held _in custody_ on their behalf.


### Claiming ownership; unify achievements
If a user has the interest and the savvy to create their own blockchain account, the system has a mechanism for them to claim their user identity in each vendor's achievement ecosystem.

Once claimed it is then possible for the user to view all of their achievements across all their gaming ecosystems in one place via an achievement-aware block explorer.


### Structure
You start by creating your **Organization** which will define its own achievements and grant them to its members or users.

Next you define whatever achievement **Categories** make sense for your game or app. Maybe you want to distinguish between "Trophies" vs "Badges" or "Solo" vs "Team" achievements.

Finally you add your **Achievements** within those Categories. "Ran 10 miles" could be a "Solo" training badge.

### Note: Blockchain storage can be expensive
The structure above writes the bare minimum data to the blockchain to minimize expenses while still ensuring that the data is human-readable. Each Achievement does have an optional `description` field so you can provide more details about the accomplishment, but this is best kept as short as possible.

### Assets
Images for each achievement are probably too much data to store on the blockchain. So instead each Organization specifies an `assetbaseurl` (e.g. "mydomainname.com/images/trophies") and then each Achievement has an `assetname` (e.g. "500_kills.png"). In this way we strike a compromise between providing nicely rendered achievement browsing without burdening game studios with excessive blockchain storage costs.

## WAX notes
Must compile with `eosio.cdt` v1.4.1. Install via Homebrew:
```
brew unlink eosio.cdt
brew install https://raw.githubusercontent.com/EOSIO/homebrew-eosio.cdt/e6fc339b845219d8bc472b7a4ad0c146bd33752a/eosio.cdt.rb
```

## Running locally
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
cleos push action achieveos addorg '["alice", "Awesome Organization"]' -p alice@active
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


## Running tests
Requirements:
* nodeos
* cleos
* python3.6+
* virtualenv

See the [EOS getting started guide](https://developers.eos.io/eosio-home/docs/setting-up-your-environment) for information on installing the EOS local dev tools.

Create a new python3 virtualenv.

Install the python dependencies:
```
pip install -r requirements.txt
```

The tests are written using [EOSFactory](https://eosfactory.io/) which makes it easy to write thorough and complex unit tests in Python. EOSFactory will launch a local test node, generate user accounts, compile the smart contract, deploy it, and then execute the unit tests.

The entire process is kicked off by a single command:
```
# cd into the 'test' dir, then run:
python test_achieveos.py

# Or if you don't need to recompile:
python test_achieveos.py -s
```
