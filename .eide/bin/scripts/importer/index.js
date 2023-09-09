(async function () {

    if (scriptArgs.length < 3) {
        throw new Error(`params too less !, format: <program> <importer> <project file path>`)
    }

    const importerName = scriptArgs[1];
    const prjFilePath = scriptArgs[2];

    const { x2js } = await import('lib/x2js/x2js.js');
    const xmlParser = new x2js({
        arrayAccessForm: 'none',
        attributePrefix: '$',
        selfClosingElements: true,
        enableToStringFunc: true,
        keepText: true
    });

    // init params
    const cont = std.loadFile(prjFilePath);
    if (cont == null) throw new Error(`file '${prjFilePath}' is not existed or can't be read !`);

    // invoke importer
    const { main } = await import(`importer/${importerName}.js`);
    const pathPart = prjFilePath.replace(/\\/g, '/').split('/');
    const prjName = pathPart.splice(pathPart.length - 1, 1)[0].replace(/\.[^\.]+$/, '');
    const prjRoot = pathPart.join('/');
    const prjInfo = main(xmlParser, cont, prjRoot, prjName);

    // print result or error
    print(JSON.stringify(prjInfo));

})().catch((err) => {
    print(`${err.name}: ${err.message}`);
    if (err.stack) print(`${err.stack}`);
});