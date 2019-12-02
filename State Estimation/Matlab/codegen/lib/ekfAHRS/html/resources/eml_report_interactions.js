// Copyright 2008-2016 The MathWorks, Inc.

// Pointer to the active file/function we're looking at.
var activeFcn = {};
// Cache of objects that are always around (i.e. contents, messages).
var staticCache = {};
// The strings that appear in the tooltips.
var tooltipStrs;
// The strings that appear in the messages (errors/warnings).  Used by message tooltips.
var messageStrs;
// Keeps track of the id of the top-most object the mouse is hovering over.
var mouseid = "";
var isIE = !!$.browser.msie;
// Indicates a function selection is currently being processed
var isLoadingFunction = false;
// Next selection in the form of a function
var nextSelectionTask = null;

// CSS selector for the currently selected expected difference message.
var currentExpDiff = "";

// Basic setup of user interactions with the text.
function setupInteractions() {

    // A blank query with no matched elements.
    queries.empty = $("empty", "<div/>");

    // The main div with the code in it.
    queries.codeDiv = $("div.code", queries.center);

    // The tag where the title of the function will be inserted.
    queries.functionTitle = $("span.functiontitle", queries.center);

    // Set up the main interactions.
    staticCache = initializeStaticCache();
    setupHovering(staticCache, queries.empty, $("tbody > tr", queries.messages));

    // Get the datatype tooltips and message tooltips in order
    initializeTooltips();
    initializeMessages();

    // Hot keys
    document.body.addEventListener("mousedown", function (evt) {
        if (evt.which == 2) {
            evt.preventDefault();
        }
    }, false);
    $(document)
        .bind('keydown',keyBindings.selectMCodeTab,     function(evt) { selectMCodeTab();      return false; })
        .bind('keydown',keyBindings.selectCallStackTab, function(evt) { selectCallStackTab();  return false; })
        .bind('keydown',keyBindings.selectCCodeTab,     function(evt) { selectCCodeTab();      return false; })

        .bind('keydown',keyBindings.selectReportTab,    function(evt) { selectReportTab();     return false; })
        .bind('keydown',keyBindings.selectCenterPanel,  function(evt) { selectCenterPanel();   return false; })

        .bind('keydown',keyBindings.selectSummaryTab,   function(evt) { selectSummaryTab();    return false; })
        .bind('keydown',keyBindings.selectMessagesTab,  function(evt) { selectMessagesTab();   return false; })
        .bind('keydown',keyBindings.selectWatchListTab, function(evt) { selectWatchListTab();  return false; })
        .bind('keydown',keyBindings.selectBuildLogTab,  function(evt) { selectBuildLogTab();   return false; })

        .bind('keydown',keyBindings.clearAllHighlights, function(evt) {clearAllHighlights();   return false; });
}

// Helper function to scroll to a variable with a specific varid, but only if the cursor
// is directly on the variable.
function scrollToTableVar(id, varid) {
    if (id == mouseid) {
        scrollToObj(queries.watchlist, $('tr.var[id="'+ varid +'"]', queries.watchTable));
    }
}

// Helper function to scroll to an mxinfo with a specific id, but only if the cursor
// is directly on the mxinfo.
function scrollToTableMxInfo(id) {
    if (id == mouseid) {
        scrollToObj(queries.watchlist, $('tr.mxinfo[id="'+ id +'"]', queries.watchTable));
    }
}

// Helper function to scroll to a message in the messages tab
function scrollToMessage(msgid) {
    var intids = decodeMessageID(msgid);
    scrollToObj(queries.messages, $('tr[id^="M' + intids[0] + 'F"]', queries.msgTable));
}

function highlightActiveFcn(isHighlight) {
    var mselector = '.fcn[id*="' + encodeFunctionIDfilter(activeFcn.id) + '"]';
    var cselector = '.ccode[id="' + activeFcn.id + '"]';
    var reportselector = '.report[id="' + activeFcn.id + '"]';
    var $active = $(mselector, queries.west).add(cselector, queries.west).add(reportselector,queries.west);
    if (isHighlight) {
        $active.addClass("activeFcn");
    } else {
        $active.removeClass("activeFcn");
    }
}

// Remove highlighting of the active function.
function unhighlightActiveFcn() {
    highlightActiveFcn(false);
}

// Highlights all instances of the active function in blue; if the active function is a MATLAB file
// (i.e. an inference file instead of a target code file), then we also have to search through the
// messages pane.
function updateActiveFunction() {
    highlightActiveFcn(true);

    if (!activeFcn.isUnderWarning) {
        queries.functionTitle.html(activeFcn.functionTitle);
    }
}

// Do some basic queries on the watch list, then add the rows.
function initializeWatchList(id) {
    activeFcn.tbody = $("> tbody", queries.watchTable);
}

// Executed every time an inference report function is brought up for the first time
// (but not for target code functions, they have another initializer).  This performs
// all of the queries to make the interactions between variables, mxinfos, messages,
// and functions within the code.
function initializeFcnData(id) {

    // This object will cache everything related to the active function.
    activeFcn.id = id;
    activeFcn.type = "m";
    activeFcn.cursorLine = null;

    activeFcn.code = queries.codeDiv.children(0);

    // Update highlighting of the active function instances.
    updateActiveFunction();

    // Expand the links to the source files.
    $(".lineno>a",queries.codeDiv).each(function() {
        var $this = $(this);
        var pieces = $this.attr("href").split(",");
        $this.attr("href", "matlab: emlcprivate('irOpenToLine','" + scriptInfoTable["fpath" + pieces[0]] + "'," + pieces[1] + ");");
    });

    // Query the code for any objects we want to highlight.
    activeFcn.vars = $(".var",queries.codeDiv);
    activeFcn.messages = $(".message",queries.codeDiv);

    // Initialize this function's cache with the static cache values.
    activeFcn.cache = initializeCache();

    // Add to the cache the hovering information.  By caching it ahead of time we get
    // much, much faster performance than recomputing these queries each time the mouse moves.
    setupHovering(activeFcn.cache, activeFcn.vars, activeFcn.messages);

    // Populate the call sites
    var $callSiteSelectTable = $("#callsiteselecttable");
    var $callSiteLabel = $("#callsiteselectlabel");
    $callSiteSelectTable.removeClass("hidden");
    var $callSiteSelect = $("#callsiteselect");
    var callSiteSelect = $callSiteSelect.get(0);
    while (callSiteSelect.hasChildNodes()) {
        callSiteSelect.removeChild(callSiteSelect.firstChild);
    }
    callSiteSelect.appendChild(new Option(localizedMessages.callSelectPrompt, "callsiteprompt", false, false));
    $callSiteLabel.addClass("filtered");
    $callSiteLabel.addClass("hidden");
    $callSiteSelect.addClass("hidden");
    callSiteSelect.disabled = true;

    try {
        var targetCallSites = functionInfoTable["tcall" + id];
        var sourceCallSites = functionInfoTable["scall" + id];
        if (targetCallSites.length != 0 || sourceCallSites.length != 0) {
            $callSiteLabel.removeClass("filtered");
            $callSiteLabel.removeClass("hidden");
            $callSiteSelect.removeClass("hidden");
            callSiteSelect.disabled = false;
            if (targetCallSites.length > 0) {
                var targetOptGroup = document.createElement("optgroup");
                targetOptGroup.label = localizedMessages.callSelectTarget;
                for (i = 0; i < targetCallSites.length; ++i) {
                    var text = targetCallSites[i][0];
                    var value = targetCallSites[i][1];
                    var option = new Option(text, value, false, false);
                    targetOptGroup.appendChild(option);
                }
                callSiteSelect.appendChild(targetOptGroup);
            }
            if (sourceCallSites.length > 0) {
                var sourceOptGroup = document.createElement("optgroup");
                sourceOptGroup.label = localizedMessages.callSelectSource;
                for (i = 0; i < sourceCallSites.length; ++i) {
                    var text = sourceCallSites[i][0];
                    var value = sourceCallSites[i][1];
                    var option = new Option(text, value, false, false);
                    sourceOptGroup.appendChild(option);
                }
                callSiteSelect.appendChild(sourceOptGroup);
            }
        }
    } catch (err) {
        // Legacy compilation report
    }
    $('a',queries.functionTitle).focus();
}

// Remove the code and variables from the DOM.
function cacheCode() {
    if (activeFcn.type == "m") {
        var codeDiscard = jQuery("<div></div>");
        activeFcn.code.appendTo(codeDiscard);

        var tbodyDiscard = jQuery("<div></div>");
        activeFcn.tbody.appendTo(tbodyDiscard);
    } else if (activeFcn.type == "c") {
        var codeDiscard = jQuery("<div></div>");
        activeFcn.code.appendTo(codeDiscard);
    } else if (activeFcn.type == "report") {
        var reportDiscard = jQuery("<div></div>");
        activeFcn.code.appendTo(reportDiscard);
    }
}

function showWatchDataLoading(start)
{
    var $lnkhide = start ? queries.watchlistlink : queries.watchloadlink;
    var $lnkshow = start ? queries.watchloadlink : queries.watchlistlink;
    var $objhide = $lnkhide.parent();
    var $objshow = $lnkshow.parent();

    $objhide.addClass('hidden');
    $objhide.removeClass('ui-tabs-selected');
    $objshow.removeClass('hidden');
    $objshow.addClass('ui-tabs-selected');
    tabLinkMouseDown($lnkshow);
}

// Load the code & watchlist separately.
// This gives better user feedback when loading large functions.
function loadWatchData(id)
{
    if (activeFcn.watchID == id) {
        return;
    }
    if (watchListTabSelected()) {
        showWatchDataLoading(true);
        scrollToObj(queries.watchlist, $(':first', queries.watchTable));
        try {
            var watchFilename = functionInfoTable["watch" + id];
            queries.watchlist.load(watchFilename, function() {
                setupWatchList();
                initializeWatchList();
                activeFcn.watchID = id;
                showWatchDataLoading(false);
                focusWatchListTab();
            });
        } catch (err) {
        }
    } else {
        initializeWatchList();
    }
}
function loadFunctionData(id,pieces)
{
    try {
        // AJAX load.
        // Once completely loaded into the DOM, execute initializeFcnData.
        activeFcn = {};
        activeFcn.functionTitle = functionInfoTable["title" + id];
        queries.functionTitle.html(activeFcn.functionTitle);

        var mcodeFilename = functionInfoTable["MATLABcode" + id];
        queries.codeDiv.load(mcodeFilename, function(responseText, textStatus, XMLHttpRequest) {
            initializeFcnData(id);
            if (pieces.length > 1) {
                // Scroll the file if requested.
                var $target;
                if (pieces[1][0] == 'F') {
                    // Scrolling to a callsite
                    $target = $('span.fcn[id="' + pieces[1] + '"]');
                    scrollToObj(queries.codeDiv, $target);
                } else if (pieces[1][0] == 'M'){
                    // Scrolling to a message
                    $target = $('span.message[id^="' + pieces[1] + 'F"]:first');
                    scrollToObj(queries.codeDiv, $target);
                    highlightCurrentMessage($target, true);
                } else {
                    // Scolling to a specific line number
                    $target = $('a[id="' + pieces[1] + '"]');
                    scrollToObj(queries.codeDiv, $target);
                }
                highlightCurrentLine($target);
                clearExpectedDifferenceHighlights();
                highlightCurrentExpectedDifference();
            }
            loadWatchData(id);
            // When load function data, add highlights for
            // single/double/expensive fixed point operations if user wants to see them
            refreshShowOptHighlight();

            if (RangeHighlighter) {
                var functionPositions = functionInfoTable["positions" + id];
                RangeHighlighter.setActiveScript(functionInfoTable["fpath" + id],
                    functionPositions[0], functionPositions[1]);
            }

            postAsyncLoad();
        });
    } catch (err) {
        postAsyncLoad();
    }
}
function makeCTitle(filename)
{
    var re = /_[cph]{1,3}.html$/;
    var suffix = filename.match(re);

    if (suffix == null) {
        re = /(_vhd|_v|_txt|_html).html$/;
        suffix = filename.match(re);
        //alert(suffix.length);
    }

    var newsuffix = suffix[0].substr(1,suffix[0].length-6);
    var displayname = filename.replace(re,'.'+newsuffix);
    var title = 'File: <span class="functionname">' + displayname + '</span>';

    return title;
}
function loadCData(filename,pieces,isUnderWarning)
{
    try {
        // Perform an AJAX load, and execute initializeCData when done.
        queries.functionTitle.html(makeCTitle(filename));
        var selector = filename + " td > *"; // C-file selector
        if (isUnderWarning) {
            initializeCData(filename,isUnderWarning);
        } else {
            queries.codeDiv.load(selector, function() {
                initializeCData(filename, isUnderWarning);
                if (pieces.length > 1) {
                    // Scroll the file if requested.
                    scrollToObj(queries.codeDiv, $('a[name="' + pieces[1] + '"]'));
                }
                postAsyncLoad();
            });
        }
    } catch (err) {
        postAsyncLoad();
    }
}

function loadReportData(filename,pieces)
{
    try {
        // Perform an AJAX load, and execute initializeReportData when done.
        var selector = filename;
        queries.codeDiv.load(selector, function() {
            initializeReportData(filename);
            if (pieces.length > 1) {
                // Scroll the file if requested.
                scrollToObj(queries.codeDiv, $('a[name="' + pieces[1] + '"]'));
            }
            postAsyncLoad();
        });
    } catch (err) {
        postAsyncLoad();
    }
}

// Shows the warning not to look at the MEX code in the code area.
function openMexWarning() {
    queries.codeDiv.load('mex_warning.html');
}

function defaultOpenFunctionOptions() {
    var options = {
        isPopState: false,
        isUnderWarning: false,
        tabId: 'm'
    };
    return options;
}

// Main function to open a file.  If id is an integer, then the function is an inference
// report function (MATLAB code).  If id is a string, then it corresponds to the filename of
// some other file to open up.  If the id is a string, then it can optionally be in the form
// of A#B, where A is the filename, and B is a tag within file A to scroll to.
function openFunction(aId, aOptions) {
    // Prevent bad ids from throwing errors.
    if (aId == null) {
        return;
    }
    if (aOptions == null) {
        aOptions = defaultOpenFunctionOptions();
    }
    // Parse the id
    var pieces = aId.split("#");
    if (pieces.length <= 0) {
        return;
    }
    var id = pieces[0];
    var intId = parseInt(id);
    if (activeFcn.id == id && intId == id) {
        // Scrolling within the current MATLAB file
        if (pieces.length > 1) {
            var $target;
            if (pieces[1][0] == 'M') {
                // Scrolling to a message
                $target = $('span.message[id^="' + pieces[1] + 'F"]:first');
            } else {
                // Scolling to a specific line number
                $target = $('a[id="' + pieces[1] + '"]');
            }
            scrollToObj(queries.codeDiv, $target);
            highlightCurrentLine($target);
        }
        return;
    } else if (aId[0] == '#' && activeFcn.isUnderWarning == aOptions.isUnderWarning) {
        // Scrolling within the current C file
        var $target = $('a[name="' + pieces[1] + '"]');
        scrollToObj(queries.codeDiv, $target);

        return;
    }

    // Remove highlighting from all links to the active function, as we're about to change it.
    unhighlightActiveFcn();

    // Update the history
    var newState = {
        id: aId,
        options: aOptions
    };
    var newHash = "#" + aId;
    if (!aOptions.isPopState) {
        try { // JxBrowser can error here.
            history.pushState(newState, aId, newHash);
        } catch (err) {
            // Log the error and keep going so that the report loads.
            console.log(err);
        }
    }

    // Save the previous function's code locations so we can call it up later if the function
    // is reopened.
    cacheCode();

    isLoadingFunction = true;

    if (intId == id) {
        // Load an inference report MATLAB file.
        unselectCCodeTab();
        loadFunctionData(id, pieces);
        setWatchListTabVisible(true);
    } else if (aOptions.tabId === "report") {
        // Load a report file
        selectCCodeTab();
        loadReportData(id, pieces);
        setWatchListTabVisible(false);
    } else if (aOptions.tabId === "mexWarning") {
        // Load the mex source code warning page
        if (!isCCodeTabSelected()) {
            selectCCodeTab();
        }
        openMexWarning();
        loadCData(id, pieces, aOptions.isUnderWarning);
        setWatchListTabVisible(false);
    } else {
        // Load a target code (C) file
        if (!isCCodeTabSelected()) {
            selectCCodeTab();
        }
        loadCData(id, pieces, aOptions.isUnderWarning);
        setWatchListTabVisible(false);
    }
}

function postAsyncLoad() {
    isLoadingFunction = false;
    if (nextSelectionTask) {
        try {
            nextSelectionTask();
        } finally {
            nextSelectionTask = null;
        }
    }
}

// Opens a function if no M function is being loaded
function openFunctionWhenIdle(aId, aOptions) {
    if (!isLoadingFunction) {
        openFunction(aId, aOptions);
    } else {
        nextSelectionTask = function() {
            openFunction(aId, aOptions);
        };
    }
}

// This function is executed each time a target code file is opened for the first time.
function initializeCData(filename, isUnderWarning) {
    // Set up the cache to store the information on this function.
    activeFcn = {};
    activeFcn.id = filename;
    activeFcn.type = "c";
    activeFcn.cursorLine = null;
    activeFcn.isUnderWarning = isUnderWarning;

    var $callSiteSelectTable = $("#callsiteselecttable");
    $callSiteSelectTable.addClass("hidden");

    // Extract the title from the C code file.
    var $title = $("h4 a", queries.codeDiv);
    var filelink = $title.html();
    filelink = functionInfoTable.htmlRoot + '../' + filelink;
    var fileshow = $title.html();
    $title.parent().remove();
    var shownFile = '';
    if (filelink != null) {
        htmlRoot = functionInfoTable.htmlRoot.replace(/'/g,"''");
        shownFile = 'File: <a href="matlab: coder.report.HTMLLinkManager.openSourceFile(\'' + htmlRoot + '\',\''+$title.attr('href')+'\')">' + fileshow + '</a>';
    }

    // Make the title for the file and insert it into the DOM.
    activeFcn.functionTitle = shownFile;
    activeFcn.code = queries.codeDiv.children(0);
    updateActiveFunction();

    if (isUnderWarning) {
        return;
    }

    // For each link in the target code file, disable the links and turn them into AJAX loads.
    $("a",queries.codeDiv).each(function() {
        if (this.getAttribute("href") != null) {
            // AJAX the links
            var $this = $(this);
            if ($this.parent().is("span")) {
                var pieces = $this.attr("href").split(",");
                $this.attr("href", "matlab: emlcprivate('irOpenToLine','" + scriptInfoTable["fpath" + pieces[0]] + "'," + pieces[1] + ");");
            } else {
                $this
                .attr("fcnid", $this.attr("href"))
                .click(function(evt) {
                    queries.ccoderows.removeClass("fcnid-hover");
                    openFunction($this.attr("href"));
                    return false; // Return false to prevent default href navigation.
                })
                .mouseover(function() {
                    // When we hover over a link, make it turn green in the contents list.
                    var id = $(this).attr("fcnid").split("#");
                    if (id != null && id.length > 0) {
                        queries.ccoderows.filter('[id="' + id[0] + '"]').addClass("fcnid-hover");
                    }
                })
                .mouseout(function() {
                    queries.ccoderows.removeClass("fcnid-hover");
                });
            }
        }
    });
}

// This function is executed each time a report page file is opened for the first time.
function initializeReportData(filename) {
    // Set up the cache to store the information on this function.
    activeFcn = {};
    activeFcn.id = filename;
    activeFcn.type = "report";
    activeFcn.cursorLine = null;
    activeFcn.isUnderWarning = false;

    var $callSiteSelectTable = $("#callsiteselecttable");
    $callSiteSelectTable.addClass("hidden");

    var $title = $("h1", queries.codeDiv);
    // Make the title for the file and insert it into the DOM.
    activeFcn.functionTitle = $title.html();
    activeFcn.code = queries.codeDiv.children(0);
    updateActiveFunction();

    // For each link in the target code file, disable the links and turn them into AJAX loads.
    $("a",queries.codeDiv).each(function() {
        if (this.getAttribute("href") != null) {
            // AJAX the links
            var $this = $(this);
            if ($this.hasClass("code2code")) {
                var str = $this.attr("href");
                str = str.substr(str.lastIndexOf("/")+1);
                $this
                .attr("fcnid", str)
                .click(function(evt) {
                    queries.ccoderows.removeClass("fcnid-hover");
                    openFunction($this.attr("href"));
                    return false; // Return false to prevent default href navigation.
                })
                .mouseover(function() {
                    // When we hover over a link, make it turn green in the contents list.
                    var id = $(this).attr("fcnid").split("#");
                    if (id != null && id.length > 0) {
                        queries.ccoderows.filter('[id="' + id[0] + '"]').addClass("fcnid-hover");
                    }
                })
                .mouseout(function() {
                    queries.ccoderows.removeClass("fcnid-hover");
                });
            }
        }
    });
}

// Code to show the tooltips
var tooltip = {};

function initializeTooltips() {
    // Lookup all tooltip strings ahead of time for faster processing
    tooltipStrs = $("div.tooltip", queries.aux);
    tooltip.parent = $('<div class="tooltip"><div class="body"></div></div>').appendTo(document.body).hide();
    tooltip.body = $('div.body', tooltip.parent);
    tooltip.counter = 1;
    $(tooltip.parent).bind('mouseenter',preventHideTooltip);
    $(tooltip.parent).bind('mouseleave',forceHideTooltip);
}

function showTooltip(evt, text) {
    forceHideTooltip();
    c = tooltip.counter + 1;
    tooltip.counter = c;
    $(evt.target).bind('mouseleave',function (evt) { hideTooltip(evt,c); });
    tooltip.body.html(text).show();
    updateTooltip(evt);
}

function preventHideTooltip()
{
    tooltip.counter = tooltip.counter + 1;
}

function forceHideTooltip()
{
    tooltip.parent.hide();
}

function hideTooltip(evt,c) {
    if (tooltip.counter <= c) {
	$(evt.target).unbind('mouseleave');
	setTimeout(function () {
	    if(tooltip.counter == c) {
		tooltip.parent.hide();
	    }
	}, 200);
	mouseid = "";
    }
}

function updateTooltip(evt) {
    if (evt && !isNaN(evt.pageX) && !isNaN(evt.pageY)) {
        var $obj = $(evt.target ? evt.target : evt.srcElement);
        var offset = $obj.offset();
        var left = offset.left + 10;
        var top = offset.top + $obj.outerHeight() + 1;

        if (left >= 0 && top >= 0) {
            tooltip.parent.css({
                left: left + 'px',
                top: top + 'px'
            }).show();
        }
    }
}


// Initialize a cache object.
function initializeStaticCache() {
    staticCache = {};
    staticCache.vars = [];
    staticCache.messages = [];
    return staticCache;
}

// Copy the static cache into the current cache.
function initializeCache() {
    cache = {};
    cache.vars = [];
    cache.messages = staticCache.messages;
    return cache;
}

// ID is formatted thus: MmFf
//  m: message number
//  f: function number
function decodeMessageID(id)
{
  var re = /M(\d+)F(\d+)/;
  var strids = re.exec(id);
  var intids = new Array(parseInt(strids[1]), parseInt(strids[2]));

  return intids;
}
// ID is formatted thus: SsTtVvUu
//  s: symbol table number
//  t: type number
//  v: value number
//  u: unique id number (ignored)
// This function extracts t and v
function convertMcodeID2TypeID(id)
{
  var ret = /T(\d+)/;
  var idt = ret.exec(id);
  var t = parseInt(idt[1]);
  var rev = /V(\d+)/;
  var idv = rev.exec(id);
  var v = 0;
  if (idv != null) {
      v = parseInt(idv[1]);
  }
  return new Array(t, v);
}
// ID is formatted thus: SsTtVvUu
//  s: symbol table number
//  t: type number
//  v: value number
//  u: unique id number (ignored)
// This function strips off VvUu
function convertMcodeID2WatchID(id)
{
    var listArray = id.split("V", 1);
    listArray = listArray[0].split("U", 1);

    return listArray[0];
}
// ID is formatted thus: FfNn:Xu
//  f: function ID
//  n: function name ID
//  u: unique ID (ignored)
// X can be one of:
//  B: used in the MATLAB code body
//  L: used as a link in the call tree (also 'l')
//  M: used in the function list
function decodeFunctionID(id)
{
  var re = /F(\d+)N(\d+)/;
  var strids = re.exec(id);

  return strids;
}
function encodeFunctionID(f,n)
{
    return 'F' + f + 'N' + n + ':';
}
// Encode only the base function ID for use in filtered searches
function encodeFunctionIDfilter(f)
{
    return 'F' + f + 'N';
}
// ID is formatted thus: SsTt
//  s: symbol table number
//  t: type number
function decodeWatchID(id)
{
  var re = /S(\d+)T(\d+)/;
  var strids = re.exec(id);

  return strids;
}
// Encode only the base watch ID for use in filtered searches.
// Converts SsTt to SsTtU
function encodeWatchIDfilter(id)
{
    var strids = decodeWatchID(id);
    return 'S' + strids[1] + 'T' + strids[2] + 'U';
}

// When a structure field is expanded, show its
// immediate descendents, and for each such descendent
// that is an expanded structure, show its descendents. Etc.
function showWatchDescendents(id)
{
    var $target = $("*." + id, queries.watchlist);
    $target.removeClass('field').show();
    $target.filter("tr.struct").each(function() {
        $thisRow = $(this);
        if ($thisRow.find("a.expanded").length > 0) {
            showWatchDescendents($thisRow.attr("id"));
        }
    });
}

function collapsibleClick(evt, $body) {
    var $obj = $(evt.target ? evt.target : evt.srcElement);
    if ($obj.is("a")) {
        var isCollapsed = $obj.hasClass("expanded");
        if (!isCollapsed) {
            $obj.addClass("expanded").removeClass("collapsed");
            $body.show();
        } else {
            $obj.removeClass("expanded").addClass("collapsed");
            $body.hide();
        }
    }
}

function methodSetClick(evt) {
    collapsibleClick(evt, queries.classlistbody);
}

function functionSetClick(evt) {
    collapsibleClick(evt, queries.functiontreebody);
}

function filterClick(evt) {
    collapsibleClick(evt, queries.filterbody);
}

// Event triggered when user clicks on "Show optimization opportunities"
function showOptClick(evt) {
    collapsibleClick(evt, queries.showoptbody);
}

// Function triggered with loading function data
function refreshShowOptHighlight() {
    toggleFcnsWithViolations($("#dblenable"), $(".dbl"), "dblHighlight");
    toggleFcnsWithViolations($("#sglenable"), $(".sgl"), "sglHighlight");
    toggleFcnsWithViolations($("#fixptenable"), $(".fixpt"), "fixptHighlight");
}

// Toggle between highlight and unhighlight of single/double/expensive fixed point operations
function toggleFcnsWithViolations($obj, $id, highlightClass) {
    // If checkbox is "checked", highlight
    if ($obj.is(":checked")) {
        $id.addClass(highlightClass);

    }
    // If checkbox is "unchecked", unhighlight
    if (!$obj.is(":checked")) {
        $id.removeClass(highlightClass);
    }
}

// Click on Double precision operations will trigger two actions
// 1. Function names with double precision operations be highlighted
// 2. Function data with double precision operations be highlighted
function dblCheckClick(evt) {
    var $obj = $(evt.target ? evt.target : evt.srcElement);
    toggleFcnsWithViolations($obj, $(".dbl"), "dblHighlight");
}

// Click on Single precision operations will trigger two actions
// 1. Function names with single precision operations be highlighted
// 2. Function data with single precision operations be highlighted
function sglCheckClick(evt) {
    var $obj = $(evt.target ? evt.target : evt.srcElement);
    toggleFcnsWithViolations($obj, $(".sgl"), "sglHighlight");
}

// Click on Expensive fixed-point operations will trigger two actions
// 1. Function names with expensive fixed-point operations be highlighted
// 2. Function data with expensive fixed-point operations be highlighted
function fixptCheckClick(evt) {
    var $obj = $(evt.target ? evt.target : evt.srcElement);
    toggleFcnsWithViolations($obj, $(".fixpt"), "fixptHighlight");
}

function ccodeClick(evt, index) {
    collapsibleClick(evt, queries.ccodelistbody[index-1]);
}

function externalLibsClick(evt) {
    collapsibleClick(evt, queries.externalLibs);
}

function watchClick(evt) {
    var $obj = $(evt.target ? evt.target : evt.srcElement);
    var isCollapsed = $obj.hasClass("expanded");
    var isExpanded = $obj.hasClass("collapsed");
    if ($obj.is("a") && (isCollapsed || isExpanded)) {
        // Collapsible row
        if (!isCollapsed) {
            $obj.addClass("expanded").removeClass("collapsed");
        } else {
            $obj.removeClass("expanded").addClass("collapsed");
        }

        var $thisRow = $obj.parent().parent();
        if ($thisRow.hasClass("struct")) {
            // Structure -- show or hide its fields
            if (isCollapsed) {
                // Hide all descendents
                var $target = $('tr[id^="' + $thisRow.attr("id") + 'F"]', queries.watchlist);
                $target.addClass('field').hide();
            } else {
                // Show immediate descendents
                showWatchDescendents($thisRow.attr("id"));
            }
            queries.watchTable.trigger('applyWidgets');
        } else {
            // FI -- show or hide the FIMATH
            var $nextRow = $thisRow.next();
            $("> td", $nextRow)[isCollapsed ? "hide" : "show"]();
        }
        return;
    }
    while (!$obj.is("tr")) {
        $obj = $obj.parent();
    }
    if ($obj.hasClass("var")) {
        var $target = $('span.var[id*="' + encodeWatchIDfilter($obj.attr("id")) + '"]:first');
        scrollToObj(queries.codeDiv, $target);
        highlightCurrentLine($target);
    }
}
function watchMouseOver(evt) {
    if (mouseid == "") {
        var $obj = $(evt.target ? evt.target : evt.srcElement);
        while (!$obj.is("tr") || $obj.attr("className") == "") {
            $obj = $obj.parent();
        }
        if ($obj.hasClass('expand-child')) {
            $obj = $obj.prev();
        }
        if ($obj.hasClass("var")) {
            mouseid = $obj.attr("id");
            addHovering(activeFcn.cache.vars[mouseid], "var-hover");
            $obj.addClass("var-hover-direct");
            if ($obj.hasClass('haschild')) {
                $obj.next().addClass("var-hover-direct");
            }
        } else if ($obj.hasClass("mxinfo")) {
            mouseid = $obj.attr("id");
            $obj.add(".mxinfo#"+mouseid, activeFcn.mxInfos)
                .addClass("mxinfo-hover");
        }
    }
}
function watchMouseOut(evt) {
    var $obj = $(evt.target ? evt.target : evt.srcElement);
    while (!$obj.is("tr") || $obj.attr("className") == "") {
        $obj = $obj.parent();
    }
    if ($obj.hasClass('expand-child')) {
        $obj = $obj.prev();
    }
    if ($obj.hasClass("var")) {
        if ($obj.attr("id") == mouseid) {
            removeHovering(activeFcn.cache.vars[mouseid], "var-hover");
            $obj.removeClass("var-hover-direct");
            if ($obj.hasClass('haschild')) {
                $obj.next().removeClass("var-hover-direct");
            }

            mouseid = "";
        }
    } else if ($obj.hasClass("mxinfo")) {
        if ($obj.attr("id") == mouseid) {
            $obj.add(".mxinfo#"+mouseid, activeFcn.mxInfos)
                .removeClass("mxinfo-hover");
            mouseid = "";
        }
    }
}

// Message list interactions
function msgHoverCssId($obj) {
    var cssid = undefined;
    if ($obj.hasClass('warning')) {
        cssid = "warning-hover";
    } else if ($obj.hasClass('info')) {
        cssid = "info-hover";
    } else if ($obj.hasClass('error') || $obj.hasClass('fatal')) {
        cssid = "error-hover";
    }
    return cssid;
}
function highlightCurrentMessage($target,highlight)
{
    var cssid = msgHoverCssId($target);
    if (cssid) {
        var msgId = $target.attr("id");
        var intids = decodeMessageID(msgId);
        var message = activeFcn.cache.messages[intids[0]];
        if (highlight) {
            addHovering(message, cssid);
        } else {
            removeHovering(message, cssid);
        }
    }
}
// This executes whenever the user clicks on an MATLAB code link in a message.
function messagesClick(evt) {
    $obj = getMessageClickTarget(evt);
    var intids = decodeMessageID($obj.attr("id"));
    openFunction(intids[1]+"#M"+intids[0]);
}
function messagesMouseOver(evt) {
    if (mouseid == "") {
        var $obj = $(evt.target ? evt.target : evt.srcElement);
        while (!$obj.is("tr")) {
            $obj = $obj.parent();
        }
        mouseid = $obj.attr("id");
        highlightCurrentMessage($obj,true);
    }
}
function messagesMouseOut(evt) {
    var $obj = $(evt.target ? evt.target : evt.srcElement);
    while (!$obj.is("tr")) {
        $obj = $obj.parent();
    }
    highlightCurrentMessage($obj,false);
    mouseid = "";
}
function clickExpectedDifferenceMessage(evt) {
    $obj = getMessageClickTarget(evt);
    clearExpectedDifferenceHighlights();
    currentExpDiff = $obj.attr("id");
    highlightExpectedDifference(currentExpDiff);
}
function highlightCurrentExpectedDifference() {
    if (currentExpDiff.trim()) {
        highlightExpectedDifference(currentExpDiff);
    }
}
function highlightExpectedDifference(ed) {
    $("." + ed).addClass("expDiffHighlight");
    $("#" + ed).addClass("expDiffHighlight");
}
function clearExpectedDifferenceHighlights() {
    $(".expDiffHighlight").removeClass("expDiffHighlight");
}
function getMessageClickTarget(evt) {
    $obj = $(evt.target ? evt.target : evt.srcElement);
    if (!$obj.attr("id"))
        $obj = $obj.parent();
    if (!$obj.attr("id"))
        $obj = $obj.parent();
    return $obj;
}
function clearAllHighlights() {
    clearExpectedDifferenceHighlights();

    // Selecting another function should not add highlights back.
    currentExpDiff = "";
}

function fcnNameHover($src, doAdd) {
    var strids = decodeFunctionID($src.attr("id"));
    var wrappedSet = $('[id*="' + encodeFunctionID(strids[1],strids[2]) + '"]', queries.west);
    if (doAdd) {
        addHovering(wrappedSet, "fcnnameid-hover");
    } else {
        removeHovering(wrappedSet, "fcnnameid-hover");
    }
}

// When the user clicks in the code, scroll to its position in the watchlist
function mcodeClick(evt) {
    $obj = $(evt.target ? evt.target : evt.srcElement);

    while (!$obj.hasClass('code')) {
        if ($obj.hasClass("fcn")) {
            // When we click on a function, it opens it up; thus, be sure to
            // unhighlight the previous function first, or else it will remain
            // highlighted if we navigate back to it.

            if ($obj.is("span")) {
                var $parent = $obj.parent();
                if ($parent.hasClass("message")) {
                    var msgid = $parent.attr("id");
                    if (msgid != 'undefined') {
                        var intids = decodeMessageID(msgid);
                        removeHovering(activeFcn.cache.messages[intids[0]], "error-hover warning-hover info-hover");
                    }
                }
                $parent.removeClass("mxinfo-hover");
            }
            fcnNameHover($obj, false);
            // Load up the file specified by the fcnID attribute
            openFunctionInFile($obj.attr("id"));
            return;
        } else if ($obj.hasClass("var")) {
            if ($obj.hasClass("var") && ($obj.hasClass("type1") || $obj.hasClass("type2"))) {
                if (watchListTabSelected()) {
                    var watchId = convertMcodeID2WatchID($obj.attr("id"));
                    scrollToTableVar($obj.attr("id"), watchId);
                }
            }
            return;
        } else if ($obj.hasClass("message")) {
            selectMessagesTab();
            scrollToMessage($obj.attr("id"));
            return;
        } else if ($obj.hasClass("shrink-button")) {
            return;
        }
        $obj = $obj.parent();
    }
}
function mcodeMouseOver(evt) {
    if (mouseid == "") {
        var $obj = $(evt.target ? evt.target : evt.srcElement);
        while (!$obj.hasClass('code')) {
            if ($obj.hasClass("var")) {
                mouseid = $obj.attr("id");
                var idstr = convertMcodeID2WatchID(mouseid);
                addHovering(activeFcn.cache.vars[idstr], "var-hover");
                var watch = document.getElementById(idstr);
                addHovering(watch, "var-hover");
                watch = document.getElementById(idstr+"C");
                if (watch) {
                    addHovering(watch, "var-hover");
                }
                $obj.addClass("var-hover-direct");

                makeVarTooltip(evt,$obj,convertMcodeID2TypeID(mouseid));
                return;
            } else if ($obj.hasClass("mxinfo")) {
                mouseid = $obj.attr("id");
                addHovering($obj, "mxinfo-hover");

                makeTypeTooltip(evt,$obj);
                return;
            } else if ($obj.hasClass("fcn")) {
                mouseid = $obj.attr("id");
                fcnNameHover($obj, true);
            } else if ($obj.hasClass("autoExtrinsicFcn")) {
                makeAutoExtrinsicTooltip(evt,$obj);
                return;
            } else if ($obj.hasClass("extrinsicFcn")) {
                makeAutoExtrinsicTooltip(evt,$obj);
                return;
            } else if ($obj.hasClass("message")) {
                mouseid = $obj.attr("id");
                var cssid = msgHoverCssId($obj);
                if (cssid) {
                    var intids = decodeMessageID(mouseid);
                    addHovering(activeFcn.cache.messages[intids[0]], cssid);
                }

                makeMsgTooltip(evt,$obj);
                return;
            }
            $obj = $obj.parent();
        }
    }
}
function mcodeMouseOut(evt) {
    var $obj = $(evt.target ? evt.target : evt.srcElement);
    while (!$obj.hasClass('code')) {
        if ($obj.hasClass("var")) {
            var idstr = convertMcodeID2WatchID($obj.attr("id"));
            removeHovering(activeFcn.cache.vars[idstr], "var-hover");
            var watch = document.getElementById(idstr);
            removeHovering(watch, "var-hover");
            watch = document.getElementById(idstr+"C");
            if (watch) {
                removeHovering(watch, "var-hover");
            }
            removeHovering($obj, "var-hover-direct");

        } else if ($obj.hasClass("mxinfo")) {
            removeHovering($obj, "mxinfo-hover");

        } else if ($obj.hasClass("fcn")) {
            fcnNameHover($obj, false);
            removeHovering($obj.parent(), "mxinfo-hover");
        } else if ($obj.hasClass("message")) {
            var cssid = msgHoverCssId($obj);
            if (cssid) {
                var intids = decodeMessageID($obj.attr("id"));
                removeHovering(activeFcn.cache.messages[intids[0]], cssid);
            }
        }
        $obj = $obj.parent();
    }
    mouseid = "";
}

function fcnListMouseOver(evt) {
    var $obj = $(evt.target ? evt.target : evt.srcElement);
    while (!$obj.is("tr") && !$obj.is("table")) {
        $obj = $obj.parent();
    }
    if ($obj.is("tr")) {
        $obj.addClass("fcnnameid-hover");
    }
}
function fcnListMouseOut(evt) {
    var $obj = $(evt.target ? evt.target : evt.srcElement);
    while (!$obj.is("tr") && !$obj.is("table")) {
        $obj = $obj.parent();
    }
    if ($obj.is("tr")) {
        $obj.removeClass("fcnnameid-hover");
    }
}
// This executes whenever the user clicks in the function list.
function fcnListClick(evt) {
    $obj = $(evt.target ? evt.target : evt.srcElement);
    while (!$obj.is("tr") && !$obj.is("table")) {
        $obj = $obj.parent();
    }
    // Load up the file specified by the fcnID attribute
    if ($obj.is("tr")) {
        $obj.removeClass("fcnnameid-hover");
        var strids = decodeFunctionID($obj.attr("id"));
        openFunction(strids[1]);
    }
}

function callTreeMouseOver(evt) {
    $obj = $(evt.target ? evt.target : evt.srcElement);
    if ($obj.is('a')) {
        $obj.addClass("fcnnameid-hover");
    }
}
function callTreeMouseOut(evt) {
    $obj = $(evt.target ? evt.target : evt.srcElement);
    if ($obj.is('a')) {
        $obj.removeClass("fcnnameid-hover");
    }
}
function openFunctionInFile(fcnId) {
    var strids = decodeFunctionID(fcnId);
    var lines = functionInfoTable["lines" + strids[1]];
    if (lines.length > 1) {
        var target = strids[1] + '#srcline' + lines[1];
        openFunction(target);
    } else {
        openFunction(strids[1]);
    }
}

// This executes whenever the user clicks in the call or class tree.
function genericTreeClick(evt, $allnodes) {
    evt.preventDefault();
    $obj = $(evt.target ? evt.target : evt.srcElement);
    while (!$obj.is("a") && !$obj.is("div")) {
        $obj = $obj.parent();
    }
    if ($obj.is('a')) {
        $obj.removeClass("fcnnameid-hover");
        // If the Enter key was pressed, rather than a MouseEvent,
        // we should uncollapse the node.
        // (Doesn't seem to be a way to distinguish the events.)
        if ($obj.parent().hasClass('collapsed')) {
            toggleCallTreeNode($obj.parent(), $allnodes);
        }
        // Load up the file specified by the fcnID attribute
        openFunctionInFile($obj.attr("id"));
    }
}
// This executes whenever the user clicks in the call tree.
function callTreeClick(evt) {
    genericTreeClick(evt, queries.allcalltreenodes);
}
// This executes whenever the user clicks in the class tree.
function classTreeClick(evt) {
    genericTreeClick(evt, queries.allclasstreenodes);
}
// This executes whenever the user clicks in the function tree.
function functionTreeClick(evt) {
    genericTreeClick(evt, queries.allfunctiontreenodes);
}
// This executes whenever the user selects a call site.
function callSiteChange(evt) {
    $obj = $(evt.target ? evt.target : evt.srcElement);
    if ($obj.is('select')) {
        var selectedIndex = $obj[0].selectedIndex;
        if (selectedIndex > 0) {
            var callSiteID = $obj[0].options[selectedIndex].value;
            openFunction(callSiteID);
        }
    }
}

function setupHovering(cache, vars, messages) {

    vars.each(function() {
        var idstr = convertMcodeID2WatchID(this.getAttribute("id"));
        if (cache.vars[idstr] == null)
            cache.vars[idstr] = [];
        cache.vars[idstr][cache.vars[idstr].length] = this;
    });
    messages.each(function() {
        var intids = decodeMessageID(this.getAttribute("id"));
        var id = intids[0];
        if (id >= cache.messages.length || cache.messages[id] == null)
            cache.messages[id] = [];
        cache.messages[id][cache.messages[id].length] = this;
    });
}

// Add classes to all matching objects when the mouse hovers over them.
function addHovering(objs, cssClass) {
    $(objs).addClass(cssClass);
    if (RangeHighlighter) {
        // Temporarily hide range highlights in the relevant region in favor of hovering
        RangeHighlighter.hide(objs);
}
}

// Remove the hovering classes.
function removeHovering(objs, cssClass) {
    $(objs).removeClass(cssClass);
    if (RangeHighlighter) {
        // Restore disabled highlights
        RangeHighlighter.show(objs);
}
}

// Highlight the current line
function highlightCurrentLine($target)
{
    if (activeFcn.cursorLine) {
        activeFcn.cursorLine.removeClass("cursorline");
        activeFcn.cursorLine = null;
    }
    var $targetLine = $target.parent();
    while ($targetLine && $targetLine[0]) {
        if ($targetLine.hasClass("srcline")) {
            $targetLine.addClass("cursorline");
            activeFcn.cursorLine = $targetLine;
            break;
        } else {
            $targetLine = $targetLine.parent();
        }
    }
}

// Make the tooltip for the auto-extrinsic function.
function makeAutoExtrinsicTooltip(evt,$obj) {
    var tooltipstr = localizedMessages.autoExtrinsicCall;
    showTooltip(evt,tooltipstr);
}

// Make all of the tooltip strings for the different mxinfos and variables
function makeTypeTooltip(evt,$obj) {
    var typestr = '<div class="label info\">' + localizedMessages.infoForExpression + '</div>';

    var re = /T(\d+):/;
    var strids = re.exec($obj.attr("id"));
    if (strids && parseInt(strids[1]) > 0)
        typestr = typestr + tooltipStrs.filter("#TT"+strids[1]).html();
    showTooltip(evt,typestr);
}
// Make the tooltip for variables.
// ids is a two-element vector containing the type and value ids.
function makeVarTooltip(evt, $obj, ids) {
    var typestr = "";
    var typeid = ids[0];
    if (typeid == 0) {
        typestr = '<div class="label type0">' + localizedMessages.infoForVarNone + '</div>';
    } else if ($obj.hasClass("type1")) {
        typestr = '<div class="label type1">' + localizedMessages.infoForVarType + '</div>';
    } else if ($obj.hasClass("type2")) {
        typestr = '<div class="label type2">' + localizedMessages.infoForVarConst + '</div>';
    }
    if (typeid > 0) {
        var ttable = tooltipStrs.filter("#TT"+typeid).clone();
        var valueid = ids[1];
        if (valueid > 0) {
            var vtable = tooltipStrs.filter("#V" + valueid);
            var vtr = $("tr", vtable).clone();
            var oldth = $("th", vtr).html();
            var name = $obj.html();
            var newth = sprintf("<a href=\"matlab: emlcprivate('irExportValue','{0}','{1}',{2})\">{3}</a>", functionInfoTable.htmlRoot, name, valueid, oldth);
            $("th", vtr).html(newth);
            $("tr:last", ttable).append(vtr);
        }
        typestr = typestr + ttable.html();
    }
    showTooltip(evt,typestr);
}
function makeMsgTooltip(evt,$obj) {
    var msgHeading = "Error Message";
    var msgClass = "error";
    if ($obj.hasClass("warning")) {
        msgHeading = "Warning Message";
        msgClass = "warning";
    } else if ($obj.hasClass("info")) {
        msgHeading = "Notice";
        msgClass = "info";
    }
    var intids = decodeMessageID($obj.attr("id"));
    var msgText = messageStrs[intids[0]];
    var typestr = '<div class="label message"><span class="' + msgClass + '">' + msgHeading + '</span><span class="msgtext"><br/>' + msgText + "</span></div>";
    showTooltip(evt,typestr);
}

// Extract all of the message strings.
function initializeMessages() {
    messageStrs = [];
    $("tr > td.text", queries.messages).each(function() {
        $this = $(this);
        var intids = decodeMessageID($this.parent().attr("id"));
        messageStrs[intids[0]] = $this.children().html();
    });
}

// A nice utility function to scroll an object.  "Obj" is the jquery object to scroll;
// "target" is the jquery of the object you want to scroll to.
// To set up scrolling to a message, do something like this: scrollToObj(queries.messages, $('tr[fcnid="' + fcnid + '"]'));
// Assuming the message table row has the fcnid="x" attribute for the function ID it refers to, that will make the table scroll
// to the correct position.  Alternatively, scrollToObj(queries.codeDiv, $('[msgid="' + msgid + '"]')); will cause
// the code to scroll to a specific message with a given id.
function scrollToObj($obj, $target) {
    if ($target == null || $target.offset() == null)
        return;

    var obj = $obj.get(0);
    var targetOffset = $target.offset()['top'] + (obj['scrollTop'] - $obj.offset()['top']);
    $obj.animate({scrollTop: targetOffset}, 'fast');
}
