![achieveos](assets/achieveos_logo_1280.png)
# Achieveos
_Achieve(ments) + EOS = Achieveos_

An open platform for Achievements/Badges/Trophies that will be preserved forever on the EOS blockchain.

## Motivation
### Achievements should have permanence
Current achievement systems are completely siloed in their own ecosystems (e.g. XBox Live gamertag, high school sports records, certifications for tech or skills training, etc) and therefore suffer from varying levels of impermanence and fragility. I can work hard to unlock a new badge in my running app ("50-Mile Club!"), but if that service shuts down, all of my badges go with it. The town secretary might be keeping track of each annual Hot Dog Eating Contest winner, but the next administration could easily lose those records as personnel change.

### Enter the blockchain
Blockchains are as close to immutable, permanent digital storage as we're ever going to get. Writing achievements to the blockchain will preserve them regardless of what happens to the organization or company that originally granted the achievement/trophy/badge.

## Achieveos overview
Achieveos is an EOS smart contract that provides a simple, open platform for any permanent achievement system to be built upon. It is structured as a series of simple tables that have database-like relationships.

You start by creating your **Organization** which will define its own achievements and grant them to its members or users. It could be a new gaming app where players can unlock trophies for completing challenges. Or a local sports team that will track its season records. Or a school's National Honor Society that will log its new inductees and other honors.

Next you define whatever achievement **Categories** make sense for your Organization. The football team might want to separate their achievements by "2019 Team Records" vs "2019 Individual Records".

And finally you add **Achievements** within those Categories. "Most Yards in a Game" could be a "2019 Team Record" shared by the whole offense while "Longest Touchdown" might be a "2019 Individual Record" held by an individual wide receiver.

Or for a gaming context maybe there's a "Challenge Badges" Category for achievements like "Killed 100 Beasts in 1 Round" while there's also a "League Trophies" Category for "Won a 8-Player Tournament".


## Running Tests
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

Execute the tests:
```
cd test
python test_achieveos.py
```
