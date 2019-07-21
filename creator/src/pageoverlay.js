let $ = require("jquery");


class PageOverlay {
    constructor() {
        const style = `
            <style>
              .page_overlay {
                display: none;
                position: absolute;
                top: 0;
                left: 0;
                width: 100%;
                height: 100%;
                background-color: rgba(0,0,0, 0.75);
                text-align: center;
                z-index: 20;
              }
              .page_overlay_popup {
                display: none;
                opacity: 0.0;
                margin: 0 auto;
                max-width: 650px;
                text-align: center;
                border-radius: 3em;
                border: 5px solid #6cc7cc;
                background-color: #eee;
                padding: 0.5em;
                padding-bottom: 1.5em;
                z-index: 30;
              }
              #page_overlay_popup_cancel_button {
                margin-right: 1em;
              }
            </style>
        `;
        const content = `
            <div class="page_overlay" id="page_overlay">
              <div class="page_overlay_popup" id="page_overlay_popup">
                <div id="page_overlay_popup_content"></div>
                <div class="float_right"><div id="page_overlay_popup_cancel_button" class="stylized_button stylized_button_tiny">cancel</div></div>
                <br/>
                <div class="clear-both"></div>
              </div>
            </div>
        `;
        document.head.insertAdjacentHTML('beforeend', style);
        document.body.insertAdjacentHTML('beforeend', content);

    }

    initPageOverlay(content_selector) {
        // Transfer the content to a new hidden div
        var $holder = $("<div/>").hide().appendTo("body");
        $(content_selector).appendTo($holder);

        // Make sure the content will be visible, even though it'll still be hidden in its parent div.
        $(content_selector).show();
    }


    showPageOverlay(content_selector) {
        // Transfer the content to the popup overlay div
        $(content_selector).appendTo($("#page_overlay_popup_content"));
        $("#page_overlay").show();

        var top_position = (window.innerHeight/2) - $("#page_overlay_popup").height();
        if (top_position < 20) { top_position = 20; }
        $("#page_overlay_popup").css({
            "position": "relative",
            "display": "block",
            "top": top_position,
        });

        window.scrollTo(0, 0);
        $("#page_overlay_popup").animate({opacity:1.0}, 500);
    }


    hidePageOverlay() {
        // Transfer the content back into a holder div
        this.initPageOverlay($("#page_overlay_popup_content").children());

        $("#page_overlay").hide();
        $("#page_overlay_popup").css({opacity:0.0});
    }



}


module.exports = PageOverlay
