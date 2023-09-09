
type VirtualRootName = '<virtual_root>'

interface VirtualFile {
    path: string;
}

interface VirtualFolder {
    name: string;
    files: VirtualFile[];
    folders: VirtualFolder[];
}

interface ProjectInfo {
    name: string;
    target?: string;
    incList: string[];
    defineList: string[];
    files: VirtualFolder;
    excludeList?: { [targetName: string]: string[] };
}
