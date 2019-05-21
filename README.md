![achieveos](assets/achieveos_logo_1280.png)
# achieveos
_Achieve(ments) + EOS = achieveos_

An open platform for Achievements/Badges/Trophies that will be preserved forever on the EOS blockchain.

## Achievements on the blockchain
Why does achieveos exist? Current achievement systems are completely siloed in their own ecosystems (e.g. XBox Live gamertag, high school sports records, certifications for tech or skills training, etc) and therefore suffer from varying levels of impermanence and fragility. I can work hard to unlock a new badge in my running app ("50-Mile Club!"), but if that service shuts down, all of my badges go with it. The town secretary might be keeping track of each annual Hot Dog Eating Contest winner, but the next administration could easily lose those records as personnel change.


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
