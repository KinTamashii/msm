"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.deactivate = exports.activate = void 0;
// The module 'vscode' contains the VS Code extensibility API
// Import the module and reference it with the alias vscode in your code below
const vscode = require("vscode");
const cp = require("child_process");
const execShell = (cmd) => new Promise((resolve, reject) => {
    cp.exec(cmd, (err, out) => {
        if (err) {
            return reject(err);
        }
        return resolve(out);
    });
});
/**
 * Create and show a StatusBarItem
 */
let msmChannel = vscode.window.createOutputChannel("now", "msm");
const runMsmExecutableCommand = async (message, command) => {
    msmChannel.clear();
    msmChannel.show();
    if (vscode.workspace.workspaceFolders === undefined) {
        msmChannel.appendLine("Error: workspaceFolder undefined!");
        return;
    }
    if (command === undefined) {
        msmChannel.appendLine("Error: command undefined!");
        return;
    }
    const output = await execShell(`cd "${vscode.workspace.workspaceFolders[0].uri.fsPath}";${command}`);
    msmChannel.appendLine(`${message}: ${command}`);
    msmChannel.append(output);
    msmChannel.appendLine("Command successfully completed.");
};
function registerMsmCommand(command, itemAlignment, itemPriority, itemText, itemTooltip, callback, thisArg) {
    const command_n = `msm.${command}Command`;
    vscode.commands.registerCommand(command_n, callback, thisArg);
    var item = vscode.window.createStatusBarItem(itemAlignment, itemPriority);
    item.text = itemText;
    item.tooltip = itemTooltip;
    item.command = command_n;
    item.show();
}
//cp -R . ~/.vscode/extensions/msm
// This method is called when your extension is activated
// Your extension is activated the very first time the command is executed
function genMsmExecutableCommand(category, type) {
    const config = vscode.workspace.getConfiguration('msm');
    if (!((category in config) && (type in config[category])))
        return undefined;
    const invokedFnDict = config[category][type];
    return `${config["executablePath"]} -verbose ${config["flags"].reduce((cmdStringAcc, curFlag) => cmdStringAcc + `-${curFlag} `, "")}${invokedFnDict.reduce((cmdStringAcc, curFnDict, index, array) => {
        var result = cmdStringAcc;
        result += `${category}=${[type, curFnDict["specification"]].join(',')} ` +
            ["input", "output", "format", "external", "id", "widths", "encodecleanflags", "language"].reduce((curParametersAcc, pName) => {
                const pQuote = (pName == "widths") ? "\\\"" : "\"";
                return curParametersAcc + ((pName in curFnDict) ?
                    `${pName}=${pQuote}${curFnDict[pName].join(`${pQuote},${pQuote}`)}${pQuote} ` :
                    "");
            }, "");
        if (index != array.length - 1)
            result += '- ';
        return result;
    }, "")}`;
}
function activate(context) {
    // Use the console to output diagnostic information (console.log) and errors (console.error)
    // This line of code will only be executed once when your extension is activated
    console.log('Congratulations, your extension "msm" is now active!');
    console.log(vscode.SemanticTokens.toString());
    //encodeItem.backgroundColor = new vscode.ThemeColor('statusBarItem.errorBackground');
    registerMsmCommand('encodeCoded', vscode.StatusBarAlignment.Left, 1, 'Encode', 'Encode MSM Files as MSBT Files', async () => {
        await runMsmExecutableCommand('Running MSM Encode Command', genMsmExecutableCommand('encode', 'coded'));
    });
    registerMsmCommand('encodeClean', vscode.StatusBarAlignment.Left, 1, 'Encode (Clean)', 'Encode TXT Files as MSBT Files.', async () => {
        await runMsmExecutableCommand('Running MSM Encode Clean Command', genMsmExecutableCommand('encode', 'clean'));
    });
    registerMsmCommand('decodeCoded', vscode.StatusBarAlignment.Left, 1, 'Decode', 'Decode MSBT Files as MSM Files.', async () => {
        await runMsmExecutableCommand('Running MSM Decode Command', genMsmExecutableCommand('decode', 'coded'));
    });
    registerMsmCommand('decodeClean', vscode.StatusBarAlignment.Left, 1, 'Decode (Clean)', 'Decode MSBT Files as TXT Files.', async () => {
        await runMsmExecutableCommand('Running MSM Decode Clean Command', genMsmExecutableCommand('decode', 'clean'));
    });
}
exports.activate = activate;
// This method is called when your extension is deactivated
function deactivate() { }
exports.deactivate = deactivate;
//"activationEvents": [], "onCommand:msm.helloWorld"
/*
{
    "command": "msm.helloWorld",
    "title": "Hello World"
}
*/ 
//# sourceMappingURL=extension.js.map