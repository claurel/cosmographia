// Simple JavaScript helper functions to maintain a stack of
// links for navigating via the back/forward buttons of a browser.

var LinkStack = new Array();
var stackIndex = 0

function setHome(u)
{
    stackIndex = 0;
    LinkStack[0] = u;
}

function navigateTo(u)
{
    stackIndex++;
    LinkStack.length = stackIndex + 1;
    LinkStack[stackIndex] = u;
}

function navigateBackward()
{
    if (stackIndex > 0)
    {
        stackIndex--;
        return LinkStack[stackIndex];
    }
    else
    {
        return "";
    }
}

function navigateForward()
{
    if (stackIndex < LinkStack.length - 1)
    {
        stackIndex++;
        return LinkStack[stackIndex];
    }
    else
    {
        return "";
    }
}

function atBeginning()
{
    return stackIndex == 0;
}

function atEnd()
{
    return stackIndex == LinkStack.length - 1
}
