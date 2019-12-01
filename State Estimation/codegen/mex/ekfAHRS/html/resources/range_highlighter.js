// Copyright 2016 The MathWorks, Inc.

//noinspection JSUnusedGlobalSymbols
var RangeHighlighter = (function() {
    //noinspection JSUnusedLocalSymbols
    var LINE_NUMBER_CLASS = "lineno",
        TAB_WRAPPER_CLASS = "tabw",
        BASE_HIGHLIGHT_CLASS="rangeHighlight",
        CSS_HIGHLIGHT_CLASS = "rangeHighlight-active",
        MSG_CONTEXT_MENU_GOTO = "contextMenuGotoExternal",
        MSG_CONTEXT_MENU_CLEAR = "contextMenuClear",
        Range = createRangeClass();
    var defaultColor = "yellow",
        indices = {},
        targetSelector,
        targetElement,
        selectionHook,
        postHighlightHook,
        childFilter = createElementFilter(),
        activeDocument,
        activeHighlight,
        styleWrapper,

        scriptContextProvider,
        currentMouseTarget,
        currentMouseContext;


    function validateSingleIndex(identifier, force) {
        if (force || !indices.hasOwnProperty(identifier)) {
            indices[identifier] = buildSingleIndex();
        }
    }

    function invalidateSingleIndex(docIdentifier) {
        if (docIdentifier) {
            delete indices[docIdentifier.script];
        }
    }

    function buildSingleIndex() {
        var data = [];
        var pointer = 0;

        function visitNode(jqElement) {
            jqElement.each(function(index, element) {
                var obj = $(element);
                if (element.nodeType === Node.TEXT_NODE) {
                    indexElement();
                } else if (element.nodeType === Node.ELEMENT_NODE) {
                    if (obj.hasClass(TAB_WRAPPER_CLASS)) {
                        indexElement(1);
                    } else {
                        visitNode(obj.contents().filter(filterNodes));
                    }
                }

                function indexElement(textLenOverride) {
                    var textLen;
                    if (textLenOverride) {
                        if (textLenOverride !== 0 && textLenOverride !== 1) {
                            throw "textLenOverride should be undefined, 0, or 1";
                        }
                        textLen = textLenOverride;
                    } else {
                        textLen = element.nodeValue.length;
                    }

                    if (textLen > 0) {
                        data.push({
                            "range": new Range(pointer, pointer + textLen),
                            "object": obj
                        });
                        pointer += textLen;
                    }
                }
            });
        }

        function filterNodes() {
            // Spans and divs without a whitelisted class are disallowed
            return this.nodeType === Node.TEXT_NODE || childFilter(this);
        }

        visitNode(targetElement.children().filter(filterNodes));
        return data;
    }

    function setHighlights(scriptIdentifier, color, ranges) {
        if(!ranges) {
            throw "At least one range must be specified in order to highlight";
        } else if (Array.isArray(ranges)) {
            // This is the branch we want
        } else {
            // Support single range and variable number of ranges
            setHighlights(scriptIdentifier, color, Array.prototype.slice.call(arguments, 2));
            return;
        }

        clearAll();

        // Validate and convert input ranges into Range objects
        var rangeObjects = new Array(ranges.length);
        for (var i = 0; i < ranges.length; i++) {
            var inputRange = ranges[i];
            rangeObjects[i] = new Range(inputRange.start, inputRange.end);
        }
        rangeObjects = sortAndMergeRanges(rangeObjects);

        var highlight = {
            script: scriptIdentifier,
            ranges: rangeObjects,
            color: color,
            applied: false,
            recent: true,
            toggleVisibility: function() {},
            cleanup: function() {}
        };

        if (selectionHook && !isActiveDocument(highlight.script, ranges.length ? ranges[0].start : null)) {
            // Attempt to switch the active script
            selectionHook(highlight.script, Math.floor((ranges[0].start + ranges[0].end) / 2));
        }

        // Now update state after possible selection
        activeHighlight = highlight;

        // Apply immediately to the DOM if the relevant script is active
        // Only run the post highlight hook for new requests with immediate effects
        applyActiveHighlight(true);
    }

    function applyActiveHighlight(runHook) {
        if (!activeHighlight || activeHighlight.applied || !activeHighlight.ranges.length ||
                !isActiveDocument(activeHighlight.script, activeHighlight.ranges[0].start)) {
            // No need to apply anything
            return false;
        }
        if (typeof runHook === "undefined") {
            runHook = false;
        }
        runHook |= activeHighlight.recent;

        // Validate and retrieve the active position-to-element mappings
        validateSingleIndex(activeHighlight.script);
        var mappings = indices[activeHighlight.script];

        // Tasks to run to effect the cleanup of this highlight from the DOM
        var cleanupTasks = [];
        var highlightElements = [];

        // Apply to the DOM
        activeHighlight.ranges.forEach(function(range) {
            // Find the first affected DOM element
            var mappingIndex = binarySearch(mappings, range, false, mappingComparator);

            if (mappingIndex >= 0 && mappingIndex < mappings.length) {
                for(; mappingIndex < mappings.length; mappingIndex++ ) {
                    var mapping = mappings[mappingIndex];
                    var intersection = range.intersection(mapping.range);
                    var highlightElement;
                    var cleanupTask;

                    if (!intersection) {
                        break;
                    }

                    if (intersection.length !== mapping.range.length) {
                        // Only part of this element's text should be highlighted
                        assert(intersection.start === mapping.range.start || intersection.end === mapping.range.end);
                        var splitResult;

                        if (intersection.start === mapping.range.start) {
                            // Only highlighting the front part
                            splitResult = splitElement(mapping.object, intersection.end - mapping.range.start);
                            highlightElement = splitResult.front;
                            cleanupTask = splitResult.cleanup;
                        } else {
                            // Only highlighting the back part
                            splitResult = splitElement(mapping.object, intersection.start - mapping.range.start);
                            highlightElement = splitResult.back;
                            cleanupTask = splitResult.cleanup;
                        }

                        assert(cleanupTask && highlightElement);
                    } else if (intersection.length > 0){
                        // "Highlight" the whole element
                        highlightElement = newTextSpan(mapping.object.get(0).nodeValue);
                        mapping.object.replaceWith(highlightElement);
                        cleanupTask = (function(startEl, endEl) {
                            return function() {
                                startEl.replaceWith(endEl);
                            };
                        })(highlightElement, mapping.object);
                    }

                    if (highlightElement) {
                        assert(highlightElement.get(0).tagName === "SPAN");
                        highlightElements.push(highlightElement.addClass(BASE_HIGHLIGHT_CLASS));

                        if (cleanupTask) {
                            assert(typeof cleanupTask === "function");
                            cleanupTasks.push(cleanupTask);
                        }
                    }
                }
            }
        });

        activeHighlight.applied = true;
        activeHighlight.recent = false;

        if (highlightElements.length > 0) {
            activeHighlight.toggleVisibility = function(visible, elementsToModulate) {
                if (elementsToModulate) {
                    elementsToModulate.find(
                            "." + BASE_HIGHLIGHT_CLASS).toggleClass(CSS_HIGHLIGHT_CLASS, visible);
                } else {
                    highlightElements.forEach(function(el) {
                        $(el).toggleClass(CSS_HIGHLIGHT_CLASS, visible);
                    });
                }
            };
            updateStyle(activeHighlight.color);
            activeHighlight.toggleVisibility(true);

            activeHighlight.cleanup = function() {
                if (activeHighlight.applied) {
                    cleanupTasks.forEach(function(task) {
                        task();
                    });
                }
                activeHighlight.applied = false;
            };

            if (runHook && postHighlightHook) {
                postHighlightHook(highlightElements);
            }
        }

        function mappingComparator(a, b) {
            return a.range.start - b.start;
        }

        function splitElement(jq, splitIndex) {
            assert(jq.get(0).nodeType === Node.TEXT_NODE,
                "This split implementation should only be used with text nodes");
            assert(jq.get(0).nodeValue.length >= splitIndex);

            var text = jq.get(0).nodeValue; // Use raw, unsanitized text (ie not text())
            var front = newTextSpan(text.substring(0, splitIndex));
            var back = newTextSpan(text.substring(splitIndex));

            jq.replaceWith(front.add(back));

            return {
                front: front,
                back: back,
                cleanup: function() {
                    // Undo the split
                    front.remove();
                    back.replaceWith(jq);
                }
            };
        }

        function newTextSpan(str) {
            return $("<span></span>").text(str);
        }

        function updateStyle(color) {
            if(!styleWrapper) {
                styleWrapper = $("<style type='text/css'></style>");
                $("head").append(styleWrapper);
            }

            var colorStr;

            if (color) {
                if (typeof color !== "string") {
                    assert(color.hasOwnProperty("length") && color.length > 2 && color.length < 5);
                    colorStr = color.length != 4 ? "rgb(" : "rgba(";
                    for (var i = 0; i < color.length; i++) {
                        colorStr += i != 3 ? Math.max(0, Math.min(color[i], 255)) : color[i] / 255;
                        if (i + 1 < color.length) {
                            colorStr += ",";
                        }
                    }
                    colorStr += ")";
                } else {
                    colorStr = color;
                }
            } else {
                colorStr = defaultColor;
            }

            styleWrapper.html("." + CSS_HIGHLIGHT_CLASS + "{background:" + colorStr + ";}");
        }
    }

    /**
     * Clear all tracked highlights (could be applied or unapplied) and remove them from the DOM
     */
    function clearAll() {
        if (activeHighlight && activeHighlight.applied) {
            activeHighlight.cleanup();
        }
        activeHighlight = null;
    }

    function setHighlightsVisible(elements, visible) {
        if(elements && activeHighlight && activeHighlight.applied) {
            activeHighlight.toggleVisibility(visible, $(elements));
        }
    }

    function onActiveScriptChange(activeScriptIdentifier, activeStart, activeEnd) {
        invalidateSingleIndex(activeDocument);
        activeDocument = activeScriptIdentifier ? {
            script: activeScriptIdentifier,
            range: typeof activeStart !== "undefined" && typeof activeEnd !== "undefined" ?
                new Range(activeStart, activeEnd) : new Range(0, Number.MAX_VALUE)
        } : null;
        invalidateSingleIndex(activeDocument);
        targetElement = targetSelector ? $(targetSelector) : null;

        if (activeHighlight) {
            activeHighlight.applied = false;
            applyActiveHighlight();
        }
    }
    
    function initialize(selector) {
        var oldTargetElement = targetElement;
        targetSelector = selector;
        targetElement = selector ? $(targetSelector) : null;

        setupMouseTracker(oldTargetElement, targetElement);
        clearAll();
        onActiveScriptChange();
    }
    
    function guaranteeInitialized(wrappedFunc) {
        var originalContext = this; // Maintain invoker's context
        return function() {
            if (!targetElement) {
                throw "RangeHighlighter not initialized";
            }
            wrappedFunc.apply(originalContext, arguments);
        };
    }

    function isActiveDocument(scriptIdentifier, representativePos) {
        if (!activeDocument && !scriptIdentifier) {
            return true; // both null
        } else if (!activeDocument || !scriptIdentifier) {
            return false; // one null;
        } else {
            // Scripts match and desired position is visible
            return activeDocument.script === scriptIdentifier && (!representativePos ||
                    activeDocument.range.contains(representativePos));
        }
    }

    function sortAndMergeRanges(rangeArray) {
        // Sort ranges
        rangeArray.sort(Range.comparator);

        // Merge overlapping ranges
        for (var i = 0; i < rangeArray.length; i++) {
            var startRange = rangeArray[i];
            var mergedStart = startRange.start;
            var mergedEnd = startRange.end;
            var mergeArrLen = 0;

            while (i + mergeArrLen + 1 < rangeArray.length) {
                var testRange = rangeArray[i + mergeArrLen + 1];

                if ((mergedStart <= testRange.start && mergedEnd >= testRange.start) ||
                    (mergedStart <= testRange.end && mergedEnd > testRange.end)) {
                    mergedStart = Math.min(testRange.start, mergedStart);
                    mergedEnd = Math.max(testRange.end, mergedEnd);
                    mergeArrLen++;
                } else {
                    break;
                }
            }

            if (mergeArrLen > 0) {
                rangeArray.splice(i, mergeArrLen + 1, new Range(mergedStart, mergedEnd));
            }
        }

        return rangeArray;
    }

    function binarySearch(arr, target, exactMatch, comparator) {
        function defaultCompare(a, b) {
            return a - b;
        }

        if (typeof comparator !== "function") {
            if (typeof comparator === "undefined") {
                comparator = defaultCompare;
            } else {
                throw "Comparator must be a function";
            }
        }

        var low = 0;
        var high = arr.length;

        while (low < high) {
            var mid = ((low + high) / 2) | 0;
            var comparison = comparator(arr[mid], target);

            if (comparison === 0) {
                return mid;
            } else if (comparison > 0) {
                // Current value is too big
                high = mid;
            } else {
                // Current value is too small
                low = mid + 1;
            }
        }

        if (!exactMatch) {
            return high - 1;
        }
    }

    function elementHasClass(el) {
        var classValue;
        if (el instanceof jQuery) {
            classValue = el.attr("class");
        } else {
            classValue = el.getAttribute("class");
        }

        if (!classValue) {
            return false;
        }

        var classes = parseClassString(classValue);
        var i;

        if (arguments.length === 2) {
            var classToMatch = arguments[1];
            for(i = 0; i < classes.length; i++) {
                if(classes[i] === classToMatch) {
                    return true;
                }
            }
        } else if (arguments.length > 2) {
            var classHash = {};
            Array.prototype.forEach.call(Array.prototype.slice.call(arguments, 1), function(clazz) {
                classHash[clazz] = true;
            });

            for(i = 0; i < classes.length; i++) {
                if(classHash[classes[i]]) {
                    return true;
                }
            }
        }

        return false;
    }

    function parseClassString(classValue) {
        return classValue.replace(/[\t\r\n]/g, " ").split(" ");
    }

    function createElementFilter() {
        return function(element) {
            // offsetParent is null whenever the element's hierarchy is not visible
            //noinspection SpellCheckingInspection
            return element.offsetParent !== null && !elementHasClass(element, LINE_NUMBER_CLASS);
        };
    }

    function setupMouseTracker(oldTarget, newTarget) {
        if (oldTarget) {
            oldTarget.unbind("mousemove.contextTracking mouseout.contextTracking");
        }
        if (newTarget) {
            newTarget.bind("mousemove.contextTracking mouseout.contextTracking", function(evt) {
                currentMouseTarget = evt.type === "mousemove" ? {
                    element: evt.target,
                    x: evt.pageX,
                    y: evt.pageY
                } : null;
                currentMouseContext = null;
            });
        }
        currentMouseTarget = null;
        currentMouseContext = null;
    }

    function determineCurrentMouseContext() {
        if (!scriptContextProvider || !currentMouseTarget) {
            return null;
        }

        var mouseElement = $(currentMouseTarget.element);

        // Determine the source line the mouse is over
        //noinspection JSUnresolvedFunction
        var lineNo = scriptContextProvider.getLineNumberFromElement(
            mouseElement, currentMouseTarget.x, currentMouseTarget.y);

        assert(typeof lineNo === "number");

        //noinspection SpellCheckingInspection
        return {
            line: lineNo,
            script: activeDocument ? activeDocument.script : "",
            onhighlight: mouseElement.hasClass(BASE_HIGHLIGHT_CLASS) || mouseElement.parents("." + BASE_HIGHLIGHT_CLASS).length > 0,
            oneditor: mouseElement.parents(targetSelector).length > 0,
            highlight: activeHighlight
        };
    }

    function assert(boolean, message) {
        if (!boolean) {
            if (message) {
                throw message;
            } else {
                throw "Assertion failed";
            }
        }
    }

    function createRangeClass() {
        var Range = function(start, end) {
            if (end < start) {
                throw "End must be greater than or equal to start";
            }
            this.start = start;
            this.end = end;
            this.length = end - start;
        };

        //noinspection JSUnusedGlobalSymbols
        Range.prototype.contains = function(other) {
            if (typeof other === "number") {
                return this.start <= other && this.end > other;
            }else {
                return this.start <= other.start && this.end >= other.end;
            }
        };

        //noinspection JSUnusedGlobalSymbols
        Range.prototype.equals = function(other) {
            return this.start === other.start && this.end === other.end;
        };

        Range.prototype.intersects = function(other) {
            return (this.start <= other.start && this.end > other.start) || (this.start <= other.end && this.end >= other.end);
        };

        Range.prototype.intersection= function(other) {
            if (this.intersects(other)) {
                return new Range(Math.max(this.start, other.start), Math.min(this.end, other.end));
            } else {
                return null;
            }
        };

        Range.prototype.compareTo = function(other) {
            var startComparison = this.start - other.start;
            if (!startComparison) {
                // Start positions are equal; compare end positions
                return this.end - other.end;
            }
            return startComparison;
        };

        Range.comparator = function(a, b) {
            return a.compareTo(b);
        } ;

        return Range;
    }

    //noinspection JSUnusedGlobalSymbols
    return {
        init: initialize,
        setActiveScript: guaranteeInitialized(onActiveScriptChange),
        highlight: guaranteeInitialized(setHighlights),
        clear: guaranteeInitialized(clearAll),
        hide: guaranteeInitialized(function(elements){
            setHighlightsVisible(elements, false);
        }),
        show: guaranteeInitialized(function(elements){
            setHighlightsVisible(elements, true);
        }),
        setSelectionHook: function(hookFun) {
            assert(hookFun === null || !hookFun || typeof hookFun === "function");
            selectionHook = hookFun;
        },
        setPostHighlightHook: function(hookFun) {
            assert(hookFun === null || !hookFun || typeof hookFun === "function");
            postHighlightHook = hookFun;
        },
        setScriptContextProvider: function(contextProvider) {
            if (contextProvider) {
                assert(typeof contextProvider === "object");
                assert(contextProvider.hasOwnProperty("getLineNumberFromElement"));
            }
            scriptContextProvider = contextProvider;
        },
        getActiveHighlight: function(json) {
            return activeHighlight ? typeof json === "boolean" && json ? JSON.stringify(activeHighlight) : activeHighlight : "";
        },
        getMouseContext: function(json) {
            if (!currentMouseContext) {
                currentMouseContext = determineCurrentMouseContext();
            }
            return currentMouseContext ? typeof json === "boolean" && json ? JSON.stringify(currentMouseContext) : currentMouseContext : "";
        }
    };
})();

