/**
 * IAR For STM8/8051 Project Importer
 * 
 * @params prjTxt {string}: project document content
 * @params prjRootPath {string}: project root
 * @params xmlParser {class x2js}: xml parser
 * 
 * @return project info {interface ProjectInfo[]}, ref {interface.ts}
*/
export function main(xmlParser, prjTxt, prjRootPath, prjName) {

    const IAR_PRJ = xmlParser.xml2js(prjTxt);

    // get project
    const PRJ = Array.isArray(IAR_PRJ['project']) ? IAR_PRJ['project'][0] : IAR_PRJ['project'];
    if (!PRJ) {
        throw new Error(`can't parse 'project' from file !`);
    }

    const PRJ_ENV = {
        PROJ_DIR: prjRootPath
    };

    const prjList = [];

    // parse base info
    PRJ.name = prjName;
    const basePrj = parseProject(prjRootPath, PRJ, JSON.parse(JSON.stringify(PRJ_ENV)));

    //
    // parse project target
    //

    const toAbsPath = (path) => {
        const m = /\$([^\s\$]+)\$/.exec(path);
        if (m && m.length > 1) {
            const keyList = m[1].split(':');
            for (const key of keyList) {
                if (PRJ_ENV[key]) {
                    const realPath = path.replace(/\$[^\s\$]+\$/, PRJ_ENV[key]);
                    return isAbsPath(realPath) ? realPath : `${prjRootPath}/${realPath}`;
                }
            }
            return path;
        }
        else {
            return isAbsPath(path) ? path : `${prjRootPath}/${path}`;
        }
    };

    if (!PRJ['configuration']) {
        throw new Error(`can't parse 'configuration' from project !`);
    }

    const targetList = toArray(PRJ['configuration']);

    for (const PRJ_TARGET of targetList) {

        const nPrj = JSON.parse(JSON.stringify(basePrj));

        // set target name
        nPrj.target = PRJ_TARGET['name'].toString();
        nPrj.name = `${nPrj.name}:${nPrj.target}`;

        const PRJ_SETTINGS = toArray(PRJ_TARGET['settings']) || [];

        const index = PRJ_SETTINGS.findIndex((setting) => setting.name.toString().startsWith('ICC'));
        if (index == -1) continue;

        const OPT_LIST = toArray(PRJ_SETTINGS[index].data.option) || [];
        for (const option of OPT_LIST) {

            if (option.state == undefined) continue;

            if (option.name.toString() == 'CCDefines') {
                nPrj.defineList = (toArray(option.state) || [])
                    .filter((item) => item.toString().trim() != '')
                    .map((item) => item.toString());
            }

            else if (option.name.toString() == 'CCIncludePath') {
                nPrj.incList = (toArray(option.state) || [])
                    .filter((item) => item.toString().trim() != '')
                    .map((item) => toAbsPath(item.toString()));
            }

            else if (option.name.toString() == 'CCIncludePath2') {
                nPrj.incList = (toArray(option.state) || [])
                    .filter((item) => item.toString().trim() != '')
                    .map((item) => toAbsPath(item.toString()));
            }
        }

        prjList.push(nPrj);
    }

    return prjList;
}

function parseProject(prjRootPath, PRJ, PRJ_ENV) {

    const vFilesRoot = {
        name: '<virtual_root>',
        files: [],
        folders: []
    };

    const result = {
        name: PRJ.name,
        incList: [],
        defineList: [],
        files: vFilesRoot,
        excludeList: {}
    };

    const toAbsPath = (path) => {
        const m = /\$([^\s\$]+)\$/.exec(path);
        if (m && m.length > 1) {
            const keyList = m[1].split(':');
            for (const key of keyList) {
                if (PRJ_ENV[key]) {
                    const realPath = path.replace(/\$[^\s\$]+\$/, PRJ_ENV[key]);
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
        if (!fileInfo.name) return;
        const absPath = toAbsPath(fileInfo.name.toString());
        parent.files.push({ path: absPath });
        if (fileInfo.excluded && fileInfo.excluded.configuration) {
            const exclCfgs = toArray(fileInfo.excluded.configuration);
            exclCfgs.forEach((target_) => {
                const target = target_.toString();
                if (!result.excludeList[target]) result.excludeList[target] = [];
                result.excludeList[target].push(absPath);
            });
        }
    };

    const pushFolder = (parent, stack, folderInfo) => {

        if (!folderInfo.name) return;

        const vFolder = {
            name: folderInfo.name.toString(),
            files: [],
            folders: []
        };

        const files = toArray(folderInfo['file']);
        if (files) {
            for (const item of files) {
                pushFile(vFolder, item);
            }
        }

        const folders = toArray(folderInfo['group']);
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
    const fStack = (toArray(PRJ['group']) || []).map((item) => {
        return {
            parent: vFilesRoot,
            folder: item
        };
    });

    while (fStack.length > 0) {
        const info = fStack.pop();
        pushFolder(info.parent, fStack, info.folder);
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
