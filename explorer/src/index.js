let $ = require("jquery");
import { Api, JsonRpc, RpcError } from 'eosjs';


var rpc = new JsonRpc('https://chain.wax.io');


// index.html
async function populateIndex(callback) {
    const resp = await rpc.get_table_rows({
        json: true,              // Get the response as json
        code: 'waxbadgesftw',     // Contract that we target
        scope: 'waxbadgesftw',         // Account that owns the data
        table: 'ecosystems',        // Table name
        limit: 10,               // Maximum number of rows that we want to get
        reverse: false,         // Optional: Get reversed data
        show_payer: false,      // Optional: Show ram payer
    });

    console.log(resp.rows);

    for(var i=0; i < resp.rows.length; i++) {
        var ecosystem = resp.rows[i];
        $("#ecosystems_list").append("<div><a href='/ecosys/" + ecosystem.key + "'>" + ecosystem.name + "</a></div>");
    }

    $("#home_container").show();
}



// ecosys.html
async function populateEcosys(key) {
    const resp = await rpc.get_table_rows({
        json: true,              // Get the response as json
        code: 'waxbadgesftw',     // Contract that we target
        scope: 'waxbadgesftw',         // Account that owns the data
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
            let $ach_template = $(".achievement_template").first().clone().appendTo($cat_template.find('.ach_list'));
            $ach_template = $ach_template.attr("id", "cat_" + i + "_ach_" + j);
            $ach_template.find(".ach_name").first().text(achievement.name);

            if (achievement.maxquantity != "0") {
                $ach_template.find(".ach_quantity").first().text("max: " + achievement.maxquantity);
            } else {
                $ach_template.find(".ach_quantity").first().text("unlimited");
            }

            $ach_template.find(".ach_description").first().text(achievement.description);
            $ach_template.show();
        }
    }

    $("#ecosystem_container").show();
}





let pathParts = window.location.pathname.split('/', 3);
let page = pathParts[1];
var param;
if (page != "") {
    param = parseInt(pathParts[2]);
}
if (page == 'ecosys') {
    populateEcosys(param);
} else {
    populateIndex();
}


$(document).ready(function() {

});




// https://stackoverflow.com/a/7732379
function getQueryParam(key) {
    key = key.replace(/[*+?^$.\[\]{}()|\\\/]/g, "\\$&"); // escape RegEx meta chars
    var match = location.search.match(new RegExp("[?&]"+key+"=([^&]+)(&|$)"));
    return match && decodeURIComponent(match[1].replace(/\+/g, " "));
}
