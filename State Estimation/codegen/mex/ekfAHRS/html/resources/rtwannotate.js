//Copyright 2010-2014 The MathWorks, Inc.

function rtwannotate_do(xmlDoc) {
    if (!xmlDoc)
        return;
    var rtwCode = document.getElementById("RTWcode");
    if (!rtwCode) {
	rtwCode = document.getElementById("codeTbl");
    }
    
    // style
    var style = xmlDoc.getElementsByTagName("style");
    if (style) {
      for (i=0;i<style.length;++i) {
        var cssCode = utils.trimText(utils.getText(style[i]));
        var styleElement = document.createElement("style");
        styleElement.type = "text/css";
        if (styleElement.styleSheet) {
          styleElement.styleSheet.cssText = cssCode;
        } else {
          styleElement.appendChild(document.createTextNode(cssCode));
        }
        document.getElementsByTagName("head")[0].appendChild(styleElement);
      }
    }
    // summary
    var summary = xmlDoc.getElementsByTagName("summary")[0];
    if (summary) {
      var summaryAnnotation = summary.getElementsByTagName("annotation")[0];
      if (summaryAnnotation) {
        var span = document.createElement("span");
        span.innerHTML = utils.trimText(utils.getText(summaryAnnotation));
        rtwCode.parentNode.insertBefore(span,rtwCode);
      }
    }
    // line
    var data = xmlDoc.getElementsByTagName("line");
    var annotationsTable = new Array();
    var defaultAnnotation;
    for (i=0;i<data.length;++i) {
      var id = data[i].getAttribute("id");
      if (id == "default") {
        defaultAnnotation = data[i].getElementsByTagName("annotation")[0];
      } else {
        annotationsTable[parseInt(id)] = data[i].getElementsByTagName("annotation");
      }
    }
    // display incode trace
    if (rtwCode.rows) {
        annotate_code_table(rtwCode);
        if (top.scrollToInitLine)
            top.scrollToInitLine();
    } else {
        annotate_code_span();
    } 
    return;

    function annotate_code_span() {
        var lines = rtwCode.childNodes;
        for (i=0;i<lines.length;++i) {
            var annotations = annotationsTable[i+1];
            if (annotations && annotations.length > 0) {  
                // first annotation
                var span = document.createElement("span");
                span.innerHTML = utils.trimText(utils.getText(annotations[0]));
                lines[i].insertBefore(span,lines[i].firstChild);
                // more annotations
                for (j=1;j<annotations.length;++j) {
                    span = document.createElement("span");
                    span.innerHTML = utils.trimText(utils.getText(annotations[j])) + "<br />";
                    // how to handle nl?
                    lines[i].appendChild(span);
                }
            } else if (defaultAnnotation) {
                // default annotation
                var newElement = document.createElement("span");
                newElement.innerHTML = utils.trimText(utils.getText(defaultAnnotation));
                lines[i].insertBefore(newElement,lines[i].firstChild);
            }
        }
    }

    function annotate_code_table(rtwCode) {
        var tr, td;
        var left_padding = "  ";
        var right_padding = "  ";
        var lines = rtwCode.rows;
        var lineNum = 0;
        if (annotationsTable.length === 0)
            return;
	var insertFunction = removeToInsertLater(rtwCode);
	rtwCode.cellSpacing = "0px";
	rtwCode.cellPadding = "0px";
        for (i=0;i<lines.length;++i) {
	    lineNum++;
	    var annotations = annotationsTable[lineNum];
	    if (annotations && annotations.length > 0) {  
	        // first annotation
	        var newElement = document.createElement("span");
    	        newElement.innerHTML = utils.trimText(utils.getText(annotations[0]));
	        td = lines[i].firstElementChild;
	        td.innerHTML = newElement.innerHTML + left_padding +td.innerHTML + right_padding;			
	        // more annotations			
	        for (j=1;j<annotations.length;++j) {
		    tr = document.createElement("tr");
		    tr.innerHTML = "<td colspan=2 style='text-align:left'>" + utils.trimText(utils.getText(annotations[j])) + "</td>"; 				
		    lines[i].parentNode.insertBefore(tr, lines[i].nextSibling);
		    ++i; // lines is a live node list. Add new line will increase the line number
	        }
	    } else if (defaultAnnotation) {
	        // default annotation
	        var newElement = document.createElement("span");
	        newElement.innerHTML = utils.trimText(utils.getText(defaultAnnotation));
	        td = lines[i].firstElementChild;
	        td.innerHTML = newElement.innerHTML + left_padding +td.innerHTML + right_padding;			
	    }
        }
	insertFunction();
    }
    function removeToInsertLater(element) {
	var parentNode = element.parentNode;
	var nextSibling = element.nextSibling;
	parentNode.removeChild(element);
	return function() {
	    if (nextSibling) {
		parentNode.insertBefore(element, nextSibling);
	    } else {
		parentNode.appendChild(element);
	    }
	};
    }
}

function rtwannotate_other(filename) {
    var span = document.createElement("span");
    span.innerHTML = "<SPAN>  Warning: Code coverage data is not loaded due to a web browser compatibility issue.</SPAN>";
    var rtwCode = document.getElementById("RTWcode");
    if (!rtwCode) {
	rtwCode = document.getElementById("codeTbl");
    }
    if (rtwCode) {
        rtwCode.parentNode.insertBefore(span,rtwCode);
    }
}

function rtwannotate(filename,options) {
    if (!utils.loadXML(filename, rtwannotate_do, options)) {
        rtwannotate_other(filename);
    }
}
