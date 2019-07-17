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
        $("#ecosystems_list").append("<div><a href='/ecosys.html?key=" + ecosystem.key + "'>" + ecosystem.name + "</a></div>");
    }
}



// ecosys.html
async function populateEcosys(callback) {
    let key = getQueryParam("key");
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

    var ecosystem = resp.rows[0];
    $("#ecosystem_name").text(ecosystem.name);
    $("#ecosystem_website").append("<a href='" + ecosystem.website + "' target='_new'>" + ecosystem.website + "</a>");
    $("#ecosystem_description").text(ecosystem.description);

    for (var i=0; i < ecosystem.categories.length; i++) {
        let category = ecosystem.categories[i];
        $("#category_list").append("<div>" + category.name + "</div>");
    }
}





$(document).ready(function() {

    console.log(window.location.href);

    if (window.location.pathname == '/ecosys.html') {
        populateEcosys();
    } else if (window.location.pathname == '/') {
        populateIndex();
    }

});




// https://stackoverflow.com/a/7732379
function getQueryParam(key) {
    key = key.replace(/[*+?^$.\[\]{}()|\\\/]/g, "\\$&"); // escape RegEx meta chars
    var match = location.search.match(new RegExp("[?&]"+key+"=([^&]+)(&|$)"));
    return match && decodeURIComponent(match[1].replace(/\+/g, " "));
}
