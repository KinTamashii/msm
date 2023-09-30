// The module 'vscode' contains the VS Code extensibility API
// Import the module and reference it with the alias vscode in your code below
import * as vscode from 'vscode';
import { MessageChannel } from 'worker_threads';
import * as cp from "child_process";
import { json } from 'stream/consumers';
import { stringify } from 'querystring';
import { format } from 'path';

const execShell = (cmd: string) =>
	new Promise<string>((resolve, reject) => {
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
let msmChannel = vscode.window.createOutputChannel("now","msm");

const runMsmExecutableCommand = async(message : string, command : string | undefined) : Promise<void> => {
	msmChannel.clear();
	msmChannel.show()
	if (vscode.workspace.workspaceFolders === undefined) {
		msmChannel.appendLine("Error: workspaceFolder undefined!")
		return;
	}
	if (command === undefined) {
		msmChannel.appendLine("Error: command undefined!");
		return;
	}
	
	const output = await execShell(`cd "${vscode.workspace.workspaceFolders[0].uri.fsPath}";${command}`);
	msmChannel.appendLine(`${message}: ${command}`)
	msmChannel.append(output);
	msmChannel.appendLine("Command successfully completed.")
}

function registerMsmCommand(
	command : string,
	itemAlignment : vscode.StatusBarAlignment,
	itemPriority : number,
	itemText : string,
	itemTooltip : string,

	callback: (...args: any[]) => any,
	thisArg?: any
) {
	
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


function genMsmExecutableCommand(category : string, type : string) : string | undefined {
	const config = vscode.workspace.getConfiguration('msm');
	if (!((category in config) && (type in config[category])))
		return undefined;
	const invokedFnDict = config[category][type];

	return `${config["executablePath"]} -verbose ${
		config["flags"].reduce((cmdStringAcc : string, curFlag : string) =>
			cmdStringAcc + `-${curFlag} `, "")
		}${
			invokedFnDict.reduce((cmdStringAcc : string, curFnDict : any, index : Number, array : Array<any>) => {
				var result = cmdStringAcc;
				result += `${category}=${[type,curFnDict["specification"]].join(',')} ` +
					["input", "output", "format", "external", "id", "widths", "encodecleanflags", "language"].reduce(
						(curParametersAcc, pName) => {
							const pQuote = (pName == "widths") ? "\\\"" : "\"";
							return curParametersAcc + (
								(pName in curFnDict) ?
									`${pName}=${pQuote}${curFnDict[pName].join(`${pQuote},${pQuote}`)}${pQuote} ` : 
									""
							);
						}, ""
					);
				
				if (index != array.length - 1) result += '- ';
				return result;
			}, "")
	}`;
}



export function activate(context: vscode.ExtensionContext) {

	// Use the console to output diagnostic information (console.log) and errors (console.error)
	// This line of code will only be executed once when your extension is activated
	console.log('Congratulations, your extension "msm" is now active!');
	console.log(vscode.SemanticTokens.toString());

	
	
	//encodeItem.backgroundColor = new vscode.ThemeColor('statusBarItem.errorBackground');
	

	registerMsmCommand(
		'encodeCoded',
		vscode.StatusBarAlignment.Left, 1,
		'Encode',
		'Encode MSM Files as MSBT Files',
		async() => {
			
			await runMsmExecutableCommand(
				'Running MSM Encode Command',
				genMsmExecutableCommand('encode', 'coded')
			);
		}
	)
	registerMsmCommand(
		'encodeClean',
		vscode.StatusBarAlignment.Left, 1,
		'Encode (Clean)',
		'Encode TXT Files as MSBT Files.',
		async() => {
			await runMsmExecutableCommand(
				'Running MSM Encode Clean Command',
				genMsmExecutableCommand('encode', 'clean')
			);
		}
	)

	registerMsmCommand(
		'decodeCoded',
		vscode.StatusBarAlignment.Left, 1,
		'Decode',
		'Decode MSBT Files as MSM Files.',
		async() => {
			await runMsmExecutableCommand(
				'Running MSM Decode Command',
				genMsmExecutableCommand('decode', 'coded')
			);
		}
	)
	registerMsmCommand(
		'decodeClean',
		vscode.StatusBarAlignment.Left, 1,
		'Decode (Clean)',
		'Decode MSBT Files as TXT Files.',
		async() => {
			await runMsmExecutableCommand(
				'Running MSM Decode Clean Command',
				genMsmExecutableCommand('decode', 'clean')
			);
		}
	)
	

}

// This method is called when your extension is deactivated
export function deactivate() {}


//"activationEvents": [], "onCommand:msm.helloWorld"
/*
{
	"command": "msm.helloWorld",
	"title": "Hello World"
}
*/