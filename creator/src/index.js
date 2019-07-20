let $ = require("jquery");

import { Api, JsonRpc, RpcError } from 'eosjs';
import { JsSignatureProvider } from 'eosjs/dist/eosjs-jssig';

let PageOverlay = require("./pageoverlay");
var pageoverlay;


var rpc = new JsonRpc('https://chain.wax.io');
var api;
var signatureProvider;


let CONTRACT = 'waxbadgesftw';


async function renderIndex() {
    console.log(ACCOUNT_NAME);

    pageoverlay.initPageOverlay($("#add_ecosys_container"));
    $("#add_ecosys_button").click(function() {
        console.log("click");
        pageoverlay.showPageOverlay($("#add_ecosys_container"));
    });

    $("#add_ecosys_submit_button").click(function() {
        let name = $("#add_ecosys_name").val();
        let description = $("#add_ecosys_description").val();
        let website = $("#add_ecosys_website").val();
        let assetbaseurl = $("#add_ecosys_assetbaseurl").val();
        let logoassetname = $("#add_ecosys_logoassetname").val();

        if (name == "") {
            alert("ecosystem name is required!");
            return;
        }
        if (website != "" && (!website.startsWith('http://') && !website.startsWith('https://'))) {
            alert("website should begin with 'http' or 'https'");
            return;
        }
        if (website == "" && assetbaseurl != "") {
            alert("website is required if assetbaseurl is specified!");
            return;
        }

        (async () => {
            const result = await api.transact(
                {
                    actions: [{
                        account: CONTRACT,
                        name: 'addecosys',
                        authorization: [{
                            actor: ACCOUNT_NAME,
                            permission: 'active',
                        }],
                        data: {
                            ecosystem_owner: ACCOUNT_NAME,
                            ecosystem_name: name,
                            description: description,
                            website: website,
                            assetbaseurl: assetbaseurl,
                            logoassetname: logoassetname
                        },
                    }]
                },
                {
                    blocksBehind: 3,
                    expireSeconds: 30,
                }
            );

            console.dir(result);
        })();

    });


    const resp = await rpc.get_table_rows({
        json: true,              // Get the response as json
        code: CONTRACT,    // Contract that we target
        scope: CONTRACT,     // Account that owns the data
        table: 'ecosystems',     // Table name
        index_position: 2,           // Table secondary key index
        key_type: 'i64',             // Secondary index type
        lower_bound: ACCOUNT_NAME,   // Table secondary key value
        upper_bound: ACCOUNT_NAME,   // Table secondary key value
        limit: 10,               // Maximum number of rows that we want to get
    });

    console.log(resp.rows);

    for(var i=0; i < resp.rows.length; i++) {
        var ecosystem = resp.rows[i];
        $("#page_index").find(".ecosystems_list").append("<div><a href='/ecosys/" + ecosystem.key + "'>" + ecosystem.name + "</a></div>");
    }

    $("#page_index").show();
}



// ecosys.html
async function renderEcosys(key) {
    const resp = await rpc.get_table_rows({
        json: true,              // Get the response as json
        code: CONTRACT,     // Contract that we target
        scope: CONTRACT,         // Account that owns the data
        table: 'ecosystems',        // Table name
        lower_bound: key,
        upper_bound: key,
        limit: 1,               // Maximum number of rows that we want to get
    });

    console.log(resp.rows);

    if (resp.rows.length == 0) {
        window.location.href = "/";
        return;
    }

    var ecosystem = resp.rows[0];
    document.title = ecosystem.name + " | WAXBadges Achievements Explorer"
    $("meta[property='og:title']").attr("content", ecosystem.name + " | WAXBadges Achievements Explorer");
    $("meta[property='og:description']").attr("content", ecosystem.description);
    $("meta[name='twitter:card'][content='summary']").text(ecosystem.description);
    $("#ecosystem_name").text(ecosystem.name);
    $("#ecosystem_website").append("<a href='" + ecosystem.website + "' target='_new'>" + ecosystem.website + "</a>");
    $("#ecosystem_description").text(ecosystem.description);

    for (var i=0; i < ecosystem.categories.length; i++) {
        let category = ecosystem.categories[i];
        console.log(category);

        let $cat_template = $(".category_template").clone().appendTo("#category_list");
        $cat_template.attr("id", "cat_" + i);
        $cat_template.find(".cat_name").text(category.name);
        $cat_template.show();

        for (var j=0; j < category.achievements.length; j++) {
            let achievement = category.achievements[j];
            console.log(achievement);
            let $ach_template = $(".achievement_template").first().clone();
            $ach_template.insertBefore($cat_template.find('.ach_list .clearfix'));
            $ach_template = $ach_template.attr("id", "cat_" + i + "_ach_" + j);
            $ach_template.find(".ach_name").text(achievement.name);
            $ach_template.find(".ach_asset").attr("src", "//" + ecosystem.assetbaseurl + "/" + achievement.assetname);

            if (achievement.maxquantity != "0") {
                $ach_template.find(".ach_quantity").text("max: " + achievement.maxquantity);
            } else {
                $ach_template.find(".ach_quantity").text("unlimited");
            }

            $ach_template.find(".ach_description").text(achievement.description);
            $ach_template.show();
        }
    }

    $("#ecosystem_container").show();
}



$(document).ready(function() {

    signatureProvider = new JsSignatureProvider([PRIVATE_KEY]);
    api = new Api({ rpc, signatureProvider });

    $(".account_name").text(ACCOUNT_NAME);

    pageoverlay = new PageOverlay();

    let pathParts = window.location.pathname.split('/', 3);
    let page = pathParts[1];
    var param;
    if (page != "") {
        param = parseInt(pathParts[2]);
    }
    if (page == 'ecosys') {
        renderEcosys(param);
    } else {
        renderIndex();
    }

});



