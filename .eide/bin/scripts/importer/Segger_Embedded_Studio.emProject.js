/**
 * Segger_Embedded_Studio Project Importer
 * 
 * @params prjTxt {string}: project document content
 * @params prjRootPath {string}: project root
 * @params xmlParser {class x2js}: xml parser
 * 
 * @return project info {interface ProjectInfo[]}, ref {interface.ts}
*/
export function main(xmlParser, prjTxt, prjRootPath) {

    const SES_PRJ = xmlParser.xml2js(prjTxt);

    // get solution
    const SLN = Array.isArray(SES_PRJ['solution']) ? SES_PRJ['solution'][0] : SES_PRJ['solution'];
    if (!SLN) {
        throw new Error(`can't parse 'solution' from file !`);
    }

    const SLN_ENV = {
        SolutionDir: prjRootPath,
        ProjectDir: prjRootPath
    };

    //
    // parse solution envs
    // 
    {
        const CFGS = toArray(SLN['configuration']);
        if (CFGS) {
            for (const CFG of CFGS) {
                if (CFG['$macros']) {
                    const envList = CFG['$macros'].split(';');
                    for (const str of envList) {
                        const m = /(?<key>[\-\w]+)=(?<val>.*)/.exec(str);
                        if (m && m.groups) {
                            SLN_ENV[m.groups['key']] = m.groups['val'];
                        }
                    }
                }
            }
        }
    }

    // get projects
    const PRJ_LIST = toArray(SLN['project']);
    if (!PRJ_LIST) {
        throw new Error(`can't parse 'project' from file !`);
    }

    const prjList = [];

    for (const PRJ of PRJ_LIST) {
        prjList.push(
            parseProject(prjRootPath, PRJ, JSON.parse(JSON.stringify(SLN_ENV)))
        );
    }

    return prjList;
}

function parseProject(prjRootPath, PRJ, SLN_ENV) {

    const vFilesRoot = {
        name: '<virtual_root>',
        files: [],
        folders: []
    };

    const result = {
        name: PRJ['$Name'],
        incList: [],
        defineList: [],
        files: vFilesRoot
    };

    const PRJ_ENV = SLN_ENV || {};

    //
    // parse project envs
    // 
    {
        const CFGS = toArray(PRJ['configuration']);
        if (CFGS) {
            for (const CFG of CFGS) {
                if (CFG['$macros']) {
                    const envList = CFG['$macros'].split(';');
                    for (const str of envList) {
                        const m = /(?<key>[\-\w]+)=(?<val>.*)/.exec(str);
                        if (m && m.groups) {
                            PRJ_ENV[m.groups['key']] = m.groups['val'];
                        }
                    }
                }
            }
        }
    }

    const toAbsPath = (path) => {
        const m = /\$\((.*?)\)/.exec(path);
        if (m && m.length > 1) {
            const keyList = m[1].split(':');
            for (const key of keyList) {
                if (PRJ_ENV[key]) {
                    const realPath = path.replace(/\$\(.*?\)/, PRJ_ENV[key]);
                    return isAbsPath(realPath) ? realPath : `${prjRootPath}/${realPath}`;
                }
            }
            return path;
        }
        else {
            return isAbsPath(path) ? path : `${prjRootPath}/${path}`;
        }
    };

    //
    // parse project files
    //

    const pushFile = (parent, fileInfo) => {
        if (!fileInfo.$file_name) return;
        parent.files.push({ path: toAbsPath(fileInfo.$file_name) });
    };

    const pushFolder = (parent, stack, folderInfo) => {

        if (!folderInfo.$Name) return;

        const vFolder = {
            name: folderInfo.$Name,
            files: [],
            folders: []
        };

        const files = toArray(folderInfo['file']);
        if (files) {
            for (const item of files) {
                pushFile(vFolder, item);
            }
        }

        const folders = toArray(folderInfo['folder']);
        if (folders) {
            for (const item of folders) {
                stack.push({
                    parent: vFolder,
                    folder: item
                });
            }
        }

        parent.folders.push(vFolder);
    };

    // push root files
    const files = toArray(PRJ['file']);
    if (files) {
        for (const item of files) {
            pushFile(vFilesRoot, item);
        }
    }

    // push root folders
    const fStack = (toArray(PRJ['folder']) || []).map((item) => {
        return {
            parent: vFilesRoot,
            folder: item
        };
    });

    while (fStack.length > 0) {
        const info = fStack.pop();
        pushFolder(info.parent, fStack, info.folder);
    }

    //
    // parse other project configs
    //

    const CFGS = toArray(PRJ['configuration']);
    if (CFGS) {
        for (const CFG of CFGS) {
            if (CFG['$c_preprocessor_definitions']) {
                const macros = CFG['$c_preprocessor_definitions'];
                result.defineList = result.defineList
                    .concat(macros.split(';')
                        .filter((macro) => macro.trim() != ''));
            }
            if (CFG['$c_user_include_directories']) {
                const incLi = CFG['$c_user_include_directories'];
                result.incList = result.incList
                    .concat(incLi.split(';')
                        .filter((path) => path.trim() != '')
                        .map((path) => toAbsPath(path)));
            }
        }
    }

    return result;
}

function toArray(obj) {
    if (!obj) return undefined;
    return Array.isArray(obj) ? obj : [obj];
}

function isAbsPath(path) {
    return path.startsWith('/') || /^[a-z]:/i.test(path);
}
