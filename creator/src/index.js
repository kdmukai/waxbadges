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
        $(".dialog_action").text("Add");
        $("#add_ecosys_name").val("");
        $("#add_ecosys_description").val("");
        $("#add_ecosys_website").val("");
        $("#add_ecosys_assetbaseurl").val("");
        $("#add_ecosys_logoassetname").val("");
        $("#add_ecosys_submit_button").text("create ecosystem");
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

        let contractFunction = "addecosys";
        let data = {
            ecosystem_owner: ACCOUNT_NAME,
            ecosystem_name: name,
            description: description,
            website: website,
            assetbaseurl: assetbaseurl,
            logoassetname: logoassetname
        }
        if ($(".dialog_action").text() == "Edit") {
            contractFunction = "editecosys";
            data["ecosystem_id"] = $("#add_ecosys_ecosystem_id").val();
        }

        pageoverlay.hidePageOverlay($("#add_ecosys_container"));
        showLog();
        writeLogCommand("push action waxbadgesftw " + contractFunction);
        showLoading();

        (async () => {
            try {
                const result = await api.transact(
                    {
                        actions: [{
                            account: CONTRACT,
                            name: contractFunction,
                            authorization: [{
                                actor: ACCOUNT_NAME,
                                permission: 'active',
                            }],
                            data: data,
                        }]
                    },
                    {
                        blocksBehind: 3,
                        expireSeconds: 30,
                    }
                );
                hideLoading();
                writeLog(JSON.stringify(result, null, 2) + "\n");

            } catch(e) {
                writeLogError(e);
            }
        })();

    });


    (async() => {
        writeLogCommand("get table ecosystems");
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

        writeLog(JSON.stringify(resp.rows, null, 2));

        for(var i=0; i < resp.rows.length; i++) {
            var ecosystem = resp.rows[i];
            $("#page_index").find(".ecosystems_list").append("<div><a href='/ecosys/" + ecosystem.key + "'>" + ecosystem.name + "</a> | <span class='stylized_button stylized_button_tiny' id='edit_ecosys_" + ecosystem.key + "'>edit</span></div>");
            // Wire up its edit click
            $("#edit_ecosys_" + ecosystem.key).click(function() {
                $(".dialog_action").text("Edit");
                $("#add_ecosys_ecosystem_id").val(ecosystem.key);
                $("#add_ecosys_name").val(ecosystem.name);
                $("#add_ecosys_description").val(ecosystem.description);
                $("#add_ecosys_website").val(ecosystem.website);
                $("#add_ecosys_assetbaseurl").val(ecosystem.assetbaseurl);
                $("#add_ecosys_logoassetname").val(ecosystem.logoassetname);
                $("#add_ecosys_submit_button").text("save changes");
                pageoverlay.showPageOverlay($("#add_ecosys_container"));
            });
        }

        hideLoading();
    })();

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



function showLog() {
    let offset = $("#hide_log_button").outerHeight() + 20;

    $("#log_stream").css({height: window.innerHeight/2 - offset});
    $("#log_container").animate(
        {
            height: window.innerHeight/2
        },
        250,
        function() {}
    );
}
function hideLog() {
    $("#log_container").animate(
        {
            height: 0
        },
        250,
        function() {}
    );

}
function writeLog(output) {
    $("#log_stream").append(output + "\n");
    $("#log_stream").animate({ scrollTop: $("#log_stream").prop("scrollHeight")}, 125);
}
function writeLogError(output) {
    writeLog("<span class='error'>" + output + "</span>");
}
function writeLogCommand(output) {
    writeLog("<span class='command'>" + output + "</span>");
}



function showLoading() {
    $("#loading_overlay").show();

    var top_position = (window.innerHeight/2) - $("#loading_container").height();
    if (top_position < 20) { top_position = 20; }
    $("#loading_container").css({
        "position": "relative",
        "display": "block",
        "top": top_position,
    });

    $("#loading_container").animate({opacity:1.0}, 500);
}
function hideLoading() {
    $("#loading_overlay").hide();
    $("#loading_container").css({opacity:0.0});
}



$(document).ready(function() {

    showLoading();

    signatureProvider = new JsSignatureProvider([PRIVATE_KEY]);
    api = new Api({ rpc, signatureProvider });

    $(".account_name").text(ACCOUNT_NAME);



    pageoverlay = new PageOverlay();
    $("#page_overlay_popup_cancel_button").click(function(){
        // Wire up the popup cancel button
        pageoverlay.hidePageOverlay();
    });

    $(document).keyup(function(e) {
        // Dismiss popup with ESC button
        if (e.keyCode == 27) {
            if ($("#page_overlay").is(":visible")) {
                pageoverlay.hidePageOverlay();
            }
        }
    });

    $(document).mouseup(function (e) {
        // Dismiss popup with click outside window
        var container = $("#page_overlay_popup");

        if (container.is(":visible") && 
            !container.is(e.target) // if the target of the click isn't the container...
            && container.has(e.target).length === 0) // ... nor a descendant of the container
        {
            pageoverlay.hidePageOverlay();
        }
    });


    $("#show_log_button").click(function(){ showLog(); });
    $("#hide_log_button").click(function(){ hideLog(); });



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



