# WAXBadges Achievements Explorer

A very simple proof-of-concept blockchain explorer to browse, view, and share your WAXBadges achievements.

[https://explorer.waxbadges.com](https://explorer.waxbadges.com)


## Introduction
Everything written to the blockchain is publicly accessible to everyone. However, interacting with the raw blockchain data is human unfriendly. Retrieving table data via the command line gets complex...

```
cleos -u https://chain.wax.io get table --index 2 --key-type i64 --lower waxbadgesftw --upper waxbadgesftw waxbadgesftw waxbadgesftw ecosystems
```

...and yields json blobs that no one really wants to sift through:

```
{
  "rows": [{
      "key": 0,
      "account": "waxbadgesftw",
      "name": "WAXBadges Genesis Campaign",
      "description": "The exclusive first achievements to ever be available on WAXBadges",
      "website": "https://github.waxbadges.com",
      "assetbaseurl": "static.waxbadges.com/assets",
      "logoassetname": "waxbadges_logo.png",
      "categories": [{
          "name": "twitter",
          "achievements": [{
              "name": "First",
              "description": "First achievement ever. First 50 to follow @WAXBadges.",
              "assetname": "ach_hand.jpg",
              "maxquantity": 50,
              "active": 1,
              "usersgranted": []
            },{
              "name": "First-ish",
              "description": "Next 100 to follow @WAXBadges.",
              "assetname": "ach_medal.jpg",
              "maxquantity": 100,
              "active": 1,
              "usersgranted": []
            }
          ]
        }
      ],
      "users": []
    }
  ],
  "more": false
}
```

## Existing block explorers are insufficient
Each contract has its own custom data table format. Regardless of whether you access the contract data via the command line or a generic block explorer, that data just isn't terribly useful without any context for understanding of how the data is organized and what it means. So each contract really needs its own custom data explorer to explain and present its data.


## Nicer UI, Social Media Savvy
The WAXBadges Achievements Explorer provides that context and makes it easy to browse the achievements hierarchy -- every Ecosystem, Achievement, and User. 

It also provides sharable links so users can brag about their latest achievements. Studios can blast out their latest amazing, awesome, fun achievement challenges to their players. [@WAXBadges](https://twitter.com/WAXBadges) can tweet out each new game and app that launches a new achievement ecosystem via the WAXBadges contract.


## Tech Overview
The fully open-sourced explorer is a ridiculously simple python Flask app deployed on AWS Lambda through Zappa, speaking to the WAX blockchain via `libeospy`. It has no database or any other data storage. It pulls everything it needs from the blockchain!

### Local dev
Create a virtualenv.

Install the python dependencies:
```
pip install -r requirements.txt
```

Run the local Flask server:
```
./run.sh
```

If you want to deploy your own version to Lambda, you'll have to do some more work. But the Zappa side of things is simple:
```
zappa init
zappa deploy <your_project_name>
```
_Note: You'll have to punch a hole through the Lambda VPS to give it external API access to talk to the blockchain (by default Lambda functions can respond to requests but cannot make their own calls out to external APIs). This requires some VPS and Security Group wizardry to connect them to a NAT gateway. Not fun. Maybe just run the code on a Raspberry Pi instead?_

