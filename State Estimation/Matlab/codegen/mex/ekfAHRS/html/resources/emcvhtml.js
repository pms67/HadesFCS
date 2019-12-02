//<script language="JavaScript">
///////////////////////////////////////////////////////////////////////////////
// Global state

var g_ErrorId = null;

///////////////////////////////////////////////////////////////////////////////
// Tree view
function swapBranch(branch){
  var objBranch = document.getElementById(branch).style;
  if(objBranch.display=="block")
    objBranch.display="none";
  else
    objBranch.display="block";
}

function swapFolder(id){
  var expandName = 'expand' + id;
  var expandImg = document.getElementById(expandName);
  var folderName = 'folder' + id;
  var folderImg = document.getElementById(folderName);
  
  var re = /(?:%..)*(\w+).gif$/;
  var fname = re.exec(expandImg.src);
  if(fname[1] == treeImgPlainPlusBase) {
    expandImg.src = treeImgPlainMinus.src;
    folderImg.src = treeImgFolderOpen.src;
  } else {
    expandImg.src = treeImgPlainPlus.src;
    folderImg.src = treeImgFolderClosed.src;
  }
}
function treeViewClick(id)
{
  swapBranch('branch' + id);
  swapFolder(id);
}
function treeViewExpand(expand)
{
  var id;
  for(id = 1; ; ++id) {
    var branchName = 'branch' + id;
    var objBranch = document.getElementById(branchName);
    var expandName = 'expand' + id;
    var expandImg = document.getElementById(expandName);
    var folderName = 'folder' + id;
    var folderImg = document.getElementById(folderName);
    
    if(objBranch == null && expandImg == null && folderImg == null) break;
    
    if(expand) {
      if(objBranch) objBranch.style.display = "block";
      if(expandImg) expandImg.src = treeImgPlainMinus.src;
      if(folderImg) folderImg.src = treeImgFolderOpen.src;
    } else {
      if(objBranch) objBranch.style.display = "none";
      if(expandImg) expandImg.src = treeImgPlainPlus.src;
      if(folderImg) folderImg.src = treeImgFolderClosed.src;
    }
  }
}
///////////////////////////////////////////////////////////////////////////////
// Tooltips
var g_ttid = null;
function tooltipId(evt)
{
  var eid = eventId(evt);
  var intlocs = decodeLocation(eid);
  var scriptId = intlocs[0];
  var messageId = intlocs[1];
  var id;
  switch (eid[0]) {
  case 'L':
    id = 'fcftooltip';
    break;
  case 'F':
    id = 'msgtooltip' + scriptId + '_' + messageId;
    break;
  default:
    id = 'msgtooltip' + scriptId + '_' + messageId;
    break;
  }

  return id;
}
function displayTT(x,y)
{
  var tt = document.getElementById(g_ttid);
  tt.style.left = x;
  tt.style.top = y;
  tt.style.visibility = 'visible';
}
function activateTT(evt)
{
  g_ttid = tooltipId(evt);
  var id = eventId(evt);
  var deltaY = 10;
  var deltaX = 0;
  if (id[0] != 'T') { deltaX = 15; deltaY = -20; }
  var x = evt.clientX + deltaX;
  var y = evt.clientY + deltaY;
  displayTT(x,y);
}
function quiesceTT(e)
{
  var tt = document.getElementById(g_ttid);
  tt.style.visibility = 'hidden';
}
///////////////////////////////////////////////////////////////////////////////
// Generic utils
function eventId(evt)
{
 var srcElement = evt.srcElement;
 if (srcElement && srcElement.id != "") return srcElement.id
 
 srcElement = evt.currentTarget;
 if (srcElement && srcElement.id != "") return srcElement.id
 
 srcElement = evt.target;
 if (srcElement && srcElement.id != "") return srcElement.id
  
 return null;
}
function cancelBubble(evt) 
{
  evt.cancelBubble=true;
  if(evt.stopPropagation) { evt.stopPropagation(); }
}
function setErrorLine(errId)
{
  g_ErrorId = errId;
}
function getErrorLine()
{
  return(g_ErrorId);
}
function hiliteErrorLine(errId)
{
  var re = /scriptArrow(\d+)_(\d+)/;
  var intlocs = re.exec(errId);
  var scriptId = intlocs[1];
  var errLine = intlocs[2];
  var lnno;
  
  var errObj = document.getElementById(errId);
  for(lnno = 1; errObj != null; ++lnno) {
    errId = 'scriptArrow' + scriptId + '_' + lnno;
    errObj = document.getElementById(errId);
    if(errObj != null) {
      errObj.style.visibility = errLine == lnno ? 'visible' : 'hidden';
    }
  }
}
function initSource() 
{
  if(parent && parent.navFrame) {
    var errId = parent.navFrame.getErrorLine();
    if(errId != null) {
      hiliteErrorLine(errId);
    }
  }
}
function initTreeView() 
{
  treeViewExpand(true);
}
///////////////////////////////////////////////////////////////////////////////
// Colorization utils
function processObject(obj)
{
  className = obj.className;

  switch(className) {
  case 'treelink':
  case 'msglink':
  case 'fcflink':
    gotoMessage(obj.id);
  default:
    return;
  } 
}
function gotoMessage(id)
{
  var re = /[^#]*/;
  var oldhtmlname = String(re.exec(parent.mainFrame.window.location.href));

  var intlocs = decodeLocation(id);
  var scriptId = intlocs[0];
  var scriptName = '_source' + scriptId + '.html';
  var re1 = /_source\d+.html/;
  var newhtmlname = oldhtmlname.replace(re1, scriptName);
  href = newhtmlname + '#scriptLine' + intlocs[0] + '_' + intlocs[4];
  parent.mainFrame.window.location.href = href;
  
  var errId = 'scriptArrow' + intlocs[0] + '_' + intlocs[4];
  parent.navFrame.setErrorLine(errId);
  if(oldhtmlname == newhtmlname) {
    parent.mainFrame.hiliteErrorLine(errId);
  }
}

// Location is formatted thus:
//  Prefix: 'T'=tooltip, 'L'=internal link, 'F'=link to editor
//  ScriptID
//  MessageID
//  MessageStartPos
//  MessageLength
//  MessageLineNo
function decodeLocation(id)
{
  var re = /\D(\d+)_(\d+)_(\d+)_(\d+)_(\d+)/;
  var strlocs = re.exec(id);
  var intlocs = new Array(parseInt(strlocs[1]), parseInt(strlocs[2]), parseInt(strlocs[3]), parseInt(strlocs[4]), parseInt(strlocs[5]));

  return intlocs;
}
function mouseOverOutObject(obj, isOver)
{
 if(obj.isActive == isOver) return;
 obj.isActive = isOver;
}
function mouseOverObject(obj)
{
 mouseOverOutObject(obj,true);
}
function mouseOutObject(obj)
{
 mouseOverOutObject(obj,false);   
}
function processMouseOver(evt)
{
  if (!evt) evt = window.event;
  cancelBubble(evt);
  id = eventId(evt);
  if(id==null) return; 

  var obj = document.getElementById(id);
  if(obj!=null) mouseOverObject(obj);
}
function processMouseOut(evt)
{
  if (!evt) evt = window.event;
  cancelBubble(evt);
  id = eventId(evt);
  if(id==null) return; 

  var obj = document.getElementById(id);
  if(obj!=null) mouseOutObject(obj);
}
function processMouseClick(evt)
{
  if (!evt) evt = window.event;
  cancelBubble(evt);
  id = eventId(evt);
  if(id==null) return; 

  var obj = document.getElementById(id);
  if(obj!=null) processObject(obj);
}
///////////////////////////////////////////////////////////////////////////////
//</script>
