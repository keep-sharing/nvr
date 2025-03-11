const readline = require('readline');
const fs = require('fs');
const path = require('path');

const MOD_E = {
    CAMERA: 0,
    DISK: 1,
    EVENT: 2,
    NETWORK: 3,
    PLAYBACK: 4,
    PTZ: 5,
    QT_CTRL: 6,
    RECORD: 7,
    SCHEDULE: 8,
    STATUS: 9,
    SYSTEM: 10,
    USER: 11,
};

const MODS = ['CAMERA', 'DISK', 'EVENT', 'NETWORK', 'PLAYBACK', 'PTZ', 'QT_CTRL', 'RECORD', 'SCHEDULE', 'STATUS', 'SYSTEM', 'USER'];

let g_arg = {
    reqName: '',
    reqId: 0,
    needSdkIn: false,
    needSdkOut: false,
    needCore: false,
    reqParams: [],
    respParams: []
};

function quest_singleline_input(prompt) {
    const rl = readline.createInterface({
        input: process.stdin,
        output: process.stdout
    });

    return new Promise((resolve, reject) => {
        rl.question(prompt, (answer) => {
            rl.close();
            resolve(answer);
        });
    });
}

function quest_multiline_input(prompt) {
    const rl = readline.createInterface({
        input: process.stdin,
        output: process.stdout
    });
    
    return new Promise((resolve, reject) => {
        let lines = [];
        rl.setPrompt(prompt);
        rl.prompt();

        rl.on('line', (line) => {
            if (line === '') {  // 如果输入为空行，则停止读取
                rl.close();
                return;
            }
            lines.push(line);
        });

        rl.on('close', () => {
            resolve(lines);
        });
    });
}

function get_param_struct_name(params)
{
    if (params.length <= 1) {
        return '/*${type}*/';
    }

    const regex = /^\s*}\s*([A-Z_]+);\s*$/;
    const res = params[params.length - 1].match(regex);
    if (res) {
        return res[1];
    }

    return '/*${type}*/';
}

function get_param_struct_members(params)
{
    const members = [];
    params.forEach((param) => {
        const regex = /^\s*([\w\s]+)\s+(\w+)(?:\[(\w*)\])?;\s*$/;
        const res = param.match(regex);
        if (res) {
            members.push({
                type: res[1],
                name: res[2],
                arrSize: res[3]
            });
        }
    });

    return members;
}

function get_req_name_without_mod()
{
    const reqNameArr = g_arg.reqName.split('.');
    reqNameArr.splice(1, 1); // remove module name
    return reqNameArr.join('_');
}

function is_get_req(reqName)
{
    return reqName.toLowerCase().indexOf('get_') !== -1;
}

function is_enum_type(type)
{
    return type.toLowerCase().indexOf('_e') !== -1;
}

function get_mod_url_filepath(mod)
{
    let filename;
    switch (mod) {
        case MOD_E.CAMERA:
            filename = 'urlCamera';
            break;
        case MOD_E.DISK:
            filename = 'urlDisk';
            break;
        case MOD_E.EVENT:
            filename = 'urlEvent';
            break;
        case MOD_E.NETWORK:
            filename = 'urlNetwork';
            break;
        case MOD_E.PLAYBACK:
            filename = 'urlPlayback';
            break;
        case MOD_E.PTZ:
            filename = 'urlPtz';
            break;
        case MOD_E.QT_CTRL:
            filename = 'urlQtCtrl';
            break;
        case MOD_E.RECORD:
            filename = 'urlRecord';
            break;
        case MOD_E.SCHEDULE:
            filename = 'urlSchedule';
            break;
        case MOD_E.STATUS:
            filename = 'urlStatus';
            break;
        case MOD_E.SYSTEM:
            filename = 'urlSystem';
            break;
        case MOD_E.USER:
            filename = 'urlUser';
            break;
        default:
            filename = 'fileNotExist';
            break;
    }

    return `src/web/webserver/${filename}`;
}

function get_mod_p2p_filepath(mod)
{
    let filename;
    switch (mod) {
        case MOD_E.CAMERA:
            filename = 'p2p_camera';
            break;
        case MOD_E.DISK:
            filename = 'p2p_disk';
            break;
        case MOD_E.EVENT:
            filename = 'p2p_event';
            break;
        case MOD_E.NETWORK:
            filename = 'p2p_network';
            break;
        case MOD_E.PLAYBACK:
            filename = 'p2p_playback';
            break;
        case MOD_E.PTZ:
            filename = 'p2p_ptz';
            break;
        case MOD_E.QT_CTRL:
            filename = 'p2p_qt_ctrl';
            break;
        case MOD_E.RECORD:
            filename = 'p2p_record';
            break;
        case MOD_E.SCHEDULE:
            filename = 'p2p_schedule';
            break;
        case MOD_E.STATUS:
            filename = 'p2p_status';
            break;
        case MOD_E.SYSTEM:
            filename = 'p2p_system';
            break;
        case MOD_E.USER:
            filename = 'p2p_user';
            break;
        default:
            filename = 'fileNotExist';
            break;
    }

    return `src/libs/sdkp2p/${filename}`;
}

function get_mod_core_filepath(mod)
{
    let filename;
    switch (mod) {
        case MOD_E.CAMERA:
            filename = 'mod_camera';
            break;
        case MOD_E.DISK:
            filename = 'mod_disk';
            break;
        case MOD_E.EVENT:
            filename = 'mod_camera'; // no mod_event
            break;
        case MOD_E.NETWORK:
            filename = 'mod_sysconf'; // no mod_network
            break;
        case MOD_E.PLAYBACK:
            filename = 'mod_retrive'; // generally written here
            break;
        case MOD_E.PTZ:
            filename = 'mod_ptz';
            break;
        case MOD_E.QT_CTRL:
            filename = 'mod_sysconf'; // no mod_qt_ctrl
            break;
        case MOD_E.RECORD:
            filename = 'mod_record';
            break;
        case MOD_E.SCHEDULE:
            filename = 'mod_schedule';
            break;
        case MOD_E.STATUS:
            filename = 'mod_camera'; // no mod_status
            break;
        case MOD_E.SYSTEM:
            filename = 'mod_sysconf'; // no mod_system
            break;
        case MOD_E.USER:
            filename = 'mod_sysconf'; // no mod_user
            break;
        default:
            filename = 'fileNotExist';
            break;
    }

    return `src/msgui/mscore/${filename}`;
}

function get_mod_from_req_name(reqName)
{
    const sMod = reqName.split('.')[1];
    switch (sMod) {
        case 'camera':
            return MOD_E.CAMERA;
        case 'event':
            return MOD_E.EVENT;
        case 'disk':
            return MOD_E.DISK;
        case 'network':
            return MOD_E.NETWORK;
        case 'playback':
            return MOD_E.PLAYBACK;
        case 'ptz':
            return MOD_E.PTZ;
        case 'qt_ctrl':
            return MOD_E.QT_CTRL;
        case 'record':
            return MOD_E.RECORD;
        case 'schedule':
            return MOD_E.SCHEDULE;
        case 'status':
            return MOD_E.STATUS;
        case 'system':
            return MOD_E.SYSTEM;
        case 'user':
            return MOD_E.USER;
        default:
            return -1;
    }
}

function update_file(sFilePath, sTargetLine, sContent)
{
    if (!sFilePath || !sTargetLine || !sContent) { 
        console.error(`file not updated, sFilePath = ${sFilePath}, sTargetLine = ${sTargetLine}, sContent = ${sContent}`);
        return -1;
    }

    const text = fs.readFileSync(sFilePath, 'utf8');
    const lines = text.split('\n');
    let targetLine = -1;

    for (let i = 0; i < lines.length; i++) {
        if (lines[i].includes(sTargetLine)) {
            targetLine = i;
            break;
        }
    }

    if (targetLine < 0) {
        console.error(`Can't find target line: ${sTargetLine}`);
        return -1;
    }

    const sContentArr = sContent.split('\n');
    lines.splice(targetLine, 0, ...sContentArr);
    
    const newText = lines.join('\n');

    fs.writeFileSync(sFilePath, newText, 'utf8');
    console.log(`${sFilePath} updated ${sTargetLine} successfully.`);

    return 0;
}

function init_url_file_macro(sFile, sTargetLine)
{
    const text = fs.readFileSync(sFile, 'utf8');
    const lines = text.split('\n');
    let targetLine = -1;

    for (let i = 0; i < lines.length; i++) {
        if (lines[i].includes(sTargetLine)) {
            targetLine = i;
            break;
        }
    }

    if (targetLine < 0) {
        console.error(`Can't find target line: ${sTargetLine}`);
        return -1;
    }

    const mod = get_mod_from_req_name(g_arg.reqName);
    const sReqName = get_req_name_without_mod();
    const macroName = `#define ${MODS[mod]}_${sReqName.toUpperCase()}`;
    const blankCnt = lines[targetLine - 1].indexOf('"') - macroName.length;
    const sBlank = ' '.repeat(blankCnt); // for format alignment

    const newMacro = `#define ${MODS[mod]}_${sReqName.toUpperCase()}${sBlank}"${g_arg.reqName}"`;
    return newMacro;
}

function init_url_file_func_define()
{
    const mod = get_mod_from_req_name(g_arg.reqName);
    const sReqName = get_req_name_without_mod();
    const sReqType = get_param_struct_name(g_arg.reqParams);
    const sRespType = get_param_struct_name(g_arg.respParams);
    let sBoaSend = '';

    let newFunc = '\n';
    if (g_arg.needSdkOut && g_arg.needCore) {
        newFunc += 'void on_url_' + sReqName.toLowerCase() + '_cb(request *req, int cnt, void *data)\n';
        newFunc += '{\n';
        newFunc += '    char resp[1024] = {0}; /* TODO: check the length enough? */\n';
        newFunc += '\n';
        newFunc += '    ware_' + MODS[mod].toLowerCase() + '_' + sReqName.toLowerCase() + '_out(req->reqUrl, data, cnt, resp, sizeof(resp));\n';
        newFunc += '    url_writeback_json(req, resp);\n';
        newFunc += '    return;\n';
        newFunc += '}\n';
        newFunc += '\n';
    }
    newFunc += 'static void on_url_' + sReqName.toLowerCase() + '(request *req, const char *pBuff)\n';
    newFunc += '{\n';
    newFunc += '    char sResp[512] = {0}; /* TODO: check the length enough? */\n';
    if (g_arg.needSdkIn) {
        newFunc += '    ' + sReqType + ' param; /* TODO: define the param variable */\n';
        newFunc += '\n';
        newFunc += '    if (ware_' + MODS[mod].toLowerCase() + '_' + sReqName.toLowerCase() + '_in(req->reqUrl, pBuff, sResp, sizeof(sResp), &param)) {\n';
        newFunc += '        url_writeback_json(req, sResp);\n';
        newFunc += '        return;\n';
        newFunc += '    }\n';
        sBoaSend = '    if (boa_send(req, REQUEST_FLAG_' + sReqName.toUpperCase() + ', &param, sizeof(' + sReqType + '), MSG_NEED_CALLBACK, sResp, sizeof(sResp))) {\n';
    } else if (g_arg.reqParams.length === 1) { // deal common param
        switch (g_arg.reqParams[0].toLowerCase()) {
            case 'int chnid;':
                newFunc += '    int chnId;\n';
                newFunc += '\n';
                newFunc += '    if (ware_common_chnid_in(req->reqUrl, pBuff, sResp, sizeof(sResp), &chnId, "chnId")) {\n';
                newFunc += '        url_writeback_json(req, sResp);\n';
                newFunc += '        return;\n';
                newFunc += '    }\n';
                sBoaSend = '    if (boa_send(req, REQUEST_FLAG_' + sReqName.toUpperCase() + ', &chnId, sizeof(int), MSG_NEED_CALLBACK, sResp, sizeof(sResp))) {\n';
                break;
            case 'uint64 chnmask;':
            case 'unsigned long long chnmask;':
                newFunc += '    Uint64 chnMask = 0;\n';
                newFunc += '\n';
                newFunc += '    if (ware_common_chnmask_in(req->reqUrl, pBuff, sResp, sizeof(sResp), &chnMask, "chnMask")) {\n';
                newFunc += '        url_writeback_json(req, sResp);\n';
                newFunc += '        return;\n';
                newFunc += '    }\n';
                sBoaSend = '    if (boa_send(req, REQUEST_FLAG_' + sReqName.toUpperCase() + ', &chnMask, sizeof(Uint64), MSG_NEED_CALLBACK, sResp, sizeof(sResp))) {\n';
                break;
            default:
                break;
        }
    }
    newFunc += '\n';
    if (g_arg.needCore) {
        newFunc += sBoaSend;
        newFunc += '        url_writeback_json(req, sResp);\n';
        newFunc += '        return;\n';
        newFunc += '    }\n';
    } else {
        if (g_arg.needSdkOut) {
            newFunc += '    ' + sRespType + ' data; /* TODO: define the param variable */\n';
            newFunc += '    memset(&data, 0, sizeof(' + sRespType + '));\n';
            newFunc += '\n';
            newFunc += '    // TODO: add your code here\n';
            newFunc += '\n';
            newFunc += '    ware_' + MODS[mod].toLowerCase() + '_' + sReqName.toLowerCase() + '_out(req->reqUrl, &data, 1, sResp, sizeof(sResp));\n';
        } else if (g_arg.respParams.length === 1) { // deal common param
            switch (g_arg.respParams[0].toLowerCase()) {
                case 'int res;':
                    newFunc += '    int res = 0;\n';
                    newFunc += '\n';
                    newFunc += '    // TODO: add your code here\n';
                    newFunc += '\n';
                    newFunc += '    ware_common_int_out(req->reqUrl, &res, 1, sResp, sizeof(sResp));\n';
                    break;
                default:
                    newFunc += '\n';
                    newFunc += '    // TODO: add your code here\n';
                    newFunc += '\n';
                    break;
            }
        }
        newFunc += '    url_writeback_json(req, sResp);\n';
    }

    newFunc += '\n';
    newFunc += '    return;\n';
    newFunc += '}';
    return newFunc;
}

function init_url_file_reg_arr()
{
    const mod = get_mod_from_req_name(g_arg.reqName);
    const sReqName = get_req_name_without_mod();

    let newReg = `    {${MODS[mod]}_${sReqName.toUpperCase()}, on_url_${sReqName.toLowerCase()}},`;
    return newReg;
}

function init_url_file_func_declare()
{
    if (!g_arg.needCore || !g_arg.needSdkOut) {
        return '';
    }

    const sReqName = get_req_name_without_mod();

    let newDeclare = `void on_url_${sReqName.toLowerCase()}_cb(request *req, int cnt, void *data);`;
    return newDeclare;
}

function init_url_file_resp_case()
{
    const sReqName = get_req_name_without_mod();
    const sRespType = get_param_struct_name(g_arg.respParams);

    let newCase = '';
    if (!g_arg.needCore) {
        return newCase;
    }

    newCase += `\n`;
    newCase += `        case RESPONSE_FLAG_${sReqName.toUpperCase()}: {\n`;
    if (g_arg.needSdkOut) {
        newCase += `            cnt =  header->size / sizeof(${sRespType});\n`;
        newCase += `            on_url_${sReqName.toLowerCase()}_cb(current, cnt, data);\n`;
    } else if (g_arg.respParams.length === 1) { // deal common param
        switch (g_arg.respParams[0].toLowerCase()) {
            case 'int res;':
                newCase += `            cnt =  header->size / sizeof(int);\n`;
                newCase += `            on_url_common_json_cb(current, cnt, data);\n`;
                break;
            default:
                break;
        }
    }
    newCase += `        }\n`;
    newCase += `        break;`;

    return newCase;
}

function update_url_file()
{
    const mod = get_mod_from_req_name(g_arg.reqName);
    const cFile = get_mod_url_filepath(mod) + '.c';
    const boaHFile = 'src/web/webserver/boa.h';
    const utilCFile = 'src/web/webserver/util.c';

    update_file(cFile, '// request macro end', init_url_file_macro(cFile, '// request macro end'));
    update_file(cFile, '// request function define end', init_url_file_func_define());
    update_file(cFile, '// request register array end', init_url_file_reg_arr());
    update_file(boaHFile, '// response function declare end', init_url_file_func_declare());
    update_file(utilCFile, '// response case end', init_url_file_resp_case());

    return;
}

function init_p2p_file_func_define()
{
    const mod = get_mod_from_req_name(g_arg.reqName);
    const sReqName = get_req_name_without_mod();
    const sReqType = get_param_struct_name(g_arg.reqParams);
    const sRespType = get_param_struct_name(g_arg.respParams);

    let paramName = '';
    let newFunc = '';
    if (g_arg.needSdkIn || !g_arg.needCore) {
        newFunc += '\n';
        newFunc += 'static void req_' + MODS[mod].toLowerCase() + '_'  + sReqName.toLowerCase() + '(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)\n';
        newFunc += '{\n';
        if (g_arg.needSdkIn) {
            newFunc += '    ' + sReqType+ ' info; /* TODO: define the param variable */\n';
            newFunc += '\n';
            newFunc += '    if (ware_' + MODS[mod].toLowerCase() + '_' + sReqName.toLowerCase() + '_in(conf->reqUrl, buf, conf->resp, conf->size, &param)) {\n';
            newFunc += '        *(conf->len) = strlen(conf->resp);\n';
            newFunc += '        return;\n';
            newFunc += '    }\n';
            newFunc += '\n';
        } else if (g_arg.reqParams.length === 1) { // deal common param
            switch (g_arg.reqParams[0].toLowerCase()) {
                case 'int chnid;':
                    newFunc += '    int chnId;\n';
                    newFunc += '\n';
                    newFunc += '    if (ware_common_chnid_in(conf->reqUrl, buf, conf->resp, conf->size, &chnId, "chnId")) {\n';
                    newFunc += '        *(conf->len) = strlen(conf->resp);\n';
                    newFunc += '        return;\n';
                    newFunc += '    }\n';
                    newFunc += '\n';
                    break;
                case 'uint64 chnmask;':
                case 'unsigned long long chnmask;':
                    newFunc += '    Uint64 chnMask;\n';
                    newFunc += '\n';
                    newFunc += '    if (ware_common_chnmask_in(conf->reqUrl, buf, conf->resp, conf->size, &chnMask, "chnMask")) {\n';
                    newFunc += '        *(conf->len) = strlen(conf->resp);\n';
                    newFunc += '        return;\n';
                    newFunc += '    }\n';
                    newFunc += '\n';
                    break;
                default:
                    break;
            }
        }
        if (g_arg.needCore) {
            if (g_arg.needSdkIn) {
                paramName = 'info';
            } else if (g_arg.reqParams.length === 1) { // deal common param
                switch (g_arg.reqParams[0].toLowerCase()) {
                    case 'int chnid;':
                        paramName = 'chnId';
                        break;
                    case 'uint64 chnmask;':
                    case 'unsigned long long chnmask;':
                        paramName = 'chnMask';
                        break;
                    default:
                        break;
                }
            }
            newFunc += '    sdkp2p_send_msg(conf, REQUEST_FLAG_' + sReqName.toUpperCase() + ', &'+ paramName +', sizeof(' + sReqType + '), SDKP2P_NEED_CALLBACK);\n';
        } else {
            newFunc += '    char reqUrl[128] = {0};\n';
            if (g_arg.needSdkOut) {
                newFunc += '    ' + sRespType + ' data; /* TODO: define the param variable */\n';
                newFunc += '    memset(&data, 0, sizeof(' + sRespType + '));\n';
                newFunc += '\n';
                newFunc += '    // TODO: add your code here\n';
                newFunc += '\n';
                newFunc += '    snprintf(reqUrl, sizeof(reqUrl), "%d", REQUEST_FLAG_' + sReqName.toUpperCase() + ');\n';
                newFunc += '    ware_' + MODS[mod].toLowerCase() + '_' + sReqName.toLowerCase() + '_out(reqUrl, &data, 1, conf->resp, conf->size);\n';
            } else if (g_arg.respParams.length === 1) { // deal common param
                switch (g_arg.respParams[0].toLowerCase()) {
                    case 'int res;':
                        newFunc += '    int res;\n';
                        newFunc += '\n';
                        newFunc += '    // TODO: add your code here\n';
                        newFunc += '\n';
                        newFunc += '    ware_common_int_out(conf->reqUrl, &res, 1, conf->resp, conf->size));\n';
                        break;
    
                    default:
                        break;
                }
            } else {
                newFunc += '    // TODO: add your code here\n';
                newFunc += '\n';
            }
            newFunc += '    *(conf->len) = strlen(conf->resp);\n';
        }
        newFunc += '\n';
        newFunc += '    return;\n';
        newFunc += '}';
    }
    if (g_arg.needSdkOut && g_arg.needCore) {
        newFunc += '\n';
        newFunc += 'void resp_' + MODS[mod].toLowerCase() + '_'  + sReqName.toLowerCase() + '(void *param, int size, char *buf, int len, int *datalen)\n';
        newFunc += '{\n';
        newFunc += '    char reqUrl[128] = {0};\n';
        newFunc += '\n';
        newFunc += '    snprintf(reqUrl, sizeof(reqUrl), "%d", REQUEST_FLAG_' + sReqName.toUpperCase() + ');\n';
        newFunc += '    ware_' + MODS[mod].toLowerCase() + '_' + sReqName.toLowerCase() + '_out(reqUrl, param, size, buf, len);\n';
        newFunc += '    *datalen = strlen(buf) + 1;\n';
        newFunc += '    return;\n';
        newFunc += '}';
    }

    return newFunc;
}

function init_p2p_file_reg_arr()
{
    const mod = get_mod_from_req_name(g_arg.reqName);
    const sReqName = get_req_name_without_mod();

    let newReg = `    {REQUEST_FLAG_${sReqName.toUpperCase()}, `;
    if (g_arg.needSdkIn || !g_arg.needCore) {
        newReg += `req_${MODS[mod].toLowerCase()}_${sReqName.toLowerCase()}, `;
    } else if (g_arg.reqParams.length === 1) { // deal common param
        switch (g_arg.reqParams[0].toLowerCase()) {
            case 'int chnid;':
                newReg += `req_common_chnid_json, `;
                break;
            case 'uint64 chnmask':
            case 'unsigned long long chnmask':
                newReg += `req_common_chnmask_json, `;
                break;
            default:
                break;
        }
    } 
    newReg += `RESPONSE_FLAG_${sReqName.toUpperCase()}, `
    if (g_arg.needCore) {
        if (g_arg.respParams.length === 1) { // deal common param
            switch (g_arg.respParams[0].toLowerCase()) {
                case 'int res;':
                    newReg += `resp_common_set_json},// ${g_arg.reqName}`;
                    break;
                default:
                    break;
            }
        } else if (g_arg.needSdkOut) {
            newReg += `resp_${MODS[mod].toLowerCase()}_${sReqName.toLowerCase()}},// ${g_arg.reqName}`;
        }
    } else {
        newReg += `NULL},// ${g_arg.reqName}`;
    }
    
    return newReg;
}

function update_p2p_file()
{
    const mod = get_mod_from_req_name(g_arg.reqName);
    const cFile = get_mod_p2p_filepath(mod) + '.c';

    update_file(cFile, '// request function define end', init_p2p_file_func_define());
    update_file(cFile, '// request register array end', init_p2p_file_reg_arr());
}

function init_core_file_func_define()
{
    if (!g_arg.needCore) {
        return '';
    }

    const sReqName = get_req_name_without_mod();
    const sReqType = get_param_struct_name(g_arg.reqParams);
    const sRespType = get_param_struct_name(g_arg.respParams);
    const reqMembers = get_param_struct_members(g_arg.reqParams);
    const respMembers = get_param_struct_members(g_arg.respParams);
    let sChnId = '';

    let newFunc = '\n';
    newFunc += 'static void on_' + sReqName.toLowerCase() + '(int reqfrom, void *param, int size, UInt64 clientId)\n';
    newFunc += '{\n';
    newFunc += '    if (!param) {\n';
    newFunc += '        sock_send_message(reqfrom, RESPONSE_FLAG_' + sReqName.toUpperCase() + ', NULL, 0, clientId);\n';
    newFunc += '        return;\n';
    newFunc += '    }\n';
    newFunc += '\n';
    newFunc += '    int ret = -1;\n';
    if (g_arg.needOvf) { 
        newFunc += '    MS_CURL_INFO info;\n';
    }
    if (g_arg.needSdkIn) {
        newFunc += '    ' + sReqType + ' *req = (' + sReqType + ' *)param;\n';
        sChnId = 'req->chnId';
    } else if (g_arg.reqParams.length === 1) { // deal common param
        switch (g_arg.reqParams[0].toLowerCase()) {
            case 'int chnid;':
                newFunc += '    int chnId = *(int *)param;\n';
                sChnId = 'chnId';
                break;
            case 'uint64 chnmask':
            case 'unsigned long long chnmask':
                newFunc += '    UInt64 chnmask = *(UInt64 *)param;\n';
                break;
            default:
                newFunc += '    ' + reqMembers[0].type + ' *req = (' + reqMembers[0].type + ' *)param;\n';
                break;
        }
    }
    if (g_arg.needSdkOut) {
        newFunc += '    ' + sRespType + ' resp;\n';
        newFunc += '    memset(&resp, 0, sizeof(' + sRespType + '));\n';
    } else if (g_arg.respParams.length === 1) { // deal common param
        newFunc += '    ' + respMembers[0].type + ' resp;\n';
        newFunc += '    memset(&resp, 0, sizeof(' + respMembers[0].type + '));\n';
    }
    newFunc += '\n';
    if (g_arg.needOvf) {
        newFunc += '    ret = get_curl_info(' + sChnId + ', &info, MS_TRUE);\n';
        newFunc += '    if (ret) {\n';
        newFunc += '        msdebug(DEBUG_INF, "get ipc curl info err, chnId = %d, ret = %d", ' + sChnId + ', ret);\n';
        newFunc += '        sock_send_message(reqfrom, RESPONSE_FLAG_' + sReqName.toUpperCase() + ', NULL, 0, clientId);\n';
        newFunc += '        return;\n';
        newFunc += '    }\n';
        newFunc += '\n';
        if (is_get_req(sReqName)) {
            newFunc += '    ret = curl_' + sReqName.toLowerCase() + '(&info, &resp);\n';
        } else { // set
            newFunc += '    ret = curl_' + sReqName.toLowerCase() + '(&info, req);\n';
        }
        newFunc += '    if (ret) {\n';
        newFunc += '        msdebug(DEBUG_INF, "' + sReqName.split('_').join(' ').toLowerCase() + ' err, chnId = %d, ret = %d", ' + sChnId + ', ret);\n';
        newFunc += '        sock_send_message(reqfrom, RESPONSE_FLAG_' + sReqName.toUpperCase() + ', NULL, 0, clientId);\n';
        newFunc += '        return;\n';
        newFunc += '    }\n';
        newFunc += '\n';
    }
    newFunc += '    // TODO: add your code here\n';
    newFunc += '\n';
    if (g_arg.needSdkOut) {
        newFunc += '    sock_send_message(reqfrom, RESPONSE_FLAG_' + sReqName.toUpperCase() + ', &resp, sizeof(' + sRespType + '), clientId);\n';
    } else if (g_arg.respParams.length === 1) { // deal common param
        switch (g_arg.respParams[0].toLowerCase()) {
            case 'int res;':
                newFunc += '    sock_send_message(reqfrom, RESPONSE_FLAG_' + sReqName.toUpperCase() + ', &ret, sizeof(int), clientId);\n';
                break;
            default:
                break;
        }
    } else {
        newFunc += '    sock_send_message(reqfrom, RESPONSE_FLAG_' + sReqName.toUpperCase() + ', NULL, 0, clientId);\n';
    }
    newFunc += '    return;\n';
    newFunc += '}';

    return newFunc;
}

function init_core_file_reg_arr()
{
    if (!g_arg.needCore) {
        return '';
    }

    const sReqName = get_req_name_without_mod();

    let newReg = `    {REQUEST_FLAG_${sReqName.toUpperCase()}, on_${sReqName.toLowerCase()}},`
    return newReg;
}

function init_core_file_request_enum()
{
    const sReqName = get_req_name_without_mod();

    let newEnum = `    REQUEST_FLAG_${sReqName.toUpperCase()} = ${g_arg.reqId},`
    return newEnum;
}

function init_core_file_response_enum()
{
    const sReqName = get_req_name_without_mod();

    let newEnum = `    RESPONSE_FLAG_${sReqName.toUpperCase()} = MAKE_RESPONSE_VALUE(REQUEST_FLAG_${sReqName.toUpperCase()}),`
    return newEnum;
}

function update_core_file()
{
    const mod = get_mod_from_req_name(g_arg.reqName);
    const cFile = get_mod_core_filepath(mod) + '.c';
    const msgHFile = 'src/public/include/msg.h';

    update_file(cFile, '// request function define end', init_core_file_func_define());
    update_file(cFile, '// request register array end', init_core_file_reg_arr());
    update_file(msgHFile, '// common request enum end', init_core_file_request_enum());
    update_file(msgHFile, '// common response enum end', init_core_file_response_enum());

    return;
}

function init_sdk_file_func_define()
{
    const mod = get_mod_from_req_name(g_arg.reqName);
    const sReqName = get_req_name_without_mod();
    const sReqType = get_param_struct_name(g_arg.reqParams);
    const sRespType = get_param_struct_name(g_arg.respParams);
    const reqMembers = get_param_struct_members(g_arg.reqParams);
    const respMembers = get_param_struct_members(g_arg.respParams);

    let newDefine = '';
    if (g_arg.needSdkIn) {
        newDefine += '\n';
        newDefine += `int ware_${MODS[mod].toLowerCase()}_${sReqName.toLowerCase()}_in(char *reqUrl, const char *jsonBuff, char *resp, int respLen, ${sReqType} *param)\n`;
        newDefine += '{\n';
        newDefine += '    sdk_request_start();\n';
        newDefine += '    memset(param, 0, sizeof(' + sReqType + '));\n';
        for (let i = 0; i < reqMembers.length; i++) {
            if (reqMembers[i].arrSize) {
                newDefine += `    // TODO: param->${reqMembers[i].name} need special deal\n`;
            } else {
                switch (reqMembers[i].type.toLowerCase()) {
                    case 'int':
                        newDefine += `    sdk_get_int(reqJson, "${reqMembers[i].name}", &param->${reqMembers[i].name}, /*NESSARY_E*/, /*NUM_LIMIT_E*/, /*min*/, /*max*/);\n`;
                        break;
                    // case 'time_t':
                    //     newDefine += `    sdk_get_int(reqJson, "${reqMembers[i].name}", (int *)&param->${reqMembers[i].name}, /*NESSARY_E*/, /*NUM_LIMIT_E*/, /*min*/, /*max*/);\n`;
                    //     break;
                    case 'char':
                        newDefine += `    sdk_get_string(reqJson, "${reqMembers[i].name}", param->${reqMembers[i].name}, /*NESSARY_E*/, /*STRING_LIMIT_E*/, /*min*/, /*max*/);\n`;
                        break;
                    case 'uint64':
                    case 'unsigned long long':
                    case 'int64':
                    case 'long long':
                        newDefine += `    sdk_get_int64(reqJson, "${reqMembers[i].name}", &param->${reqMembers[i].name}, /*NESSARY_E*/, /*STRING_LIMIT_E*/, /*min*/, /*max*/);\n`;
                        break;
                    case 'double':
                        newDefine += `    sdk_get_double(reqJson, "${reqMembers[i].name}", &param->${reqMembers[i].name}, /*NESSARY_E*/, /*NUM_LIMIT_E*/, /*min*/, /*max*/);\n`;
                        break;
                    default:
                        if (is_enum_type(reqMembers[i].type)) {
                            newDefine += `    sdk_get_int(reqJson, "${reqMembers[i].name}", (int *)&param->${reqMembers[i].name}, /*NESSARY_E*/, /*NUM_LIMIT_E*/, /*min*/, /*max*/);\n`;
                        } else {
                            newDefine += `    // TODO: param->${reqMembers[i].name} need special deal\n`;
                        }
                }
            }
        }
        // newDefine += '\n';
        // newDefine += '    // TODO: add your code here\n';
        // newDefine += '\n';
        newDefine += '    sdk_request_end();\n';
        newDefine += '    return 0;\n';
        newDefine += '}';
    }
    if (g_arg.needSdkOut) {
        newDefine += '\n';
        newDefine += `int ware_${MODS[mod].toLowerCase()}_${sReqName.toLowerCase()}_out(char *reqUrl, void *data, int cnt, char *resp, int respLen)\n`;
        newDefine += '{\n';
        newDefine += '    ' + sRespType + ' *pInfo = (' + sRespType + ' *)data;\n';
        newDefine += '\n';
        newDefine += '    sdk_judge_null(data, cnt);\n';
        newDefine += '    sdk_response_start();\n';
        for (let i = 0; i < respMembers.length; i++) {
            if (respMembers[i].arrSize) {
                newDefine += `    // TODO: pInfo->${respMembers[i].name} need special deal\n`;
            } else {
                switch (respMembers[i].type.toLowerCase()) {
                    case 'int':
                        newDefine += `    sdk_set_int(dataJson, "${respMembers[i].name}", pInfo->${respMembers[i].name});\n`;
                        break;
                    case 'time_t':
                        newDefine += `    sdk_set_int(dataJson, "${respMembers[i].name}", (int)(pInfo->${respMembers[i].name}));\n`;
                        break;
                    case 'char':
                        newDefine += `    sdk_set_string(dataJson, "${respMembers[i].name}", pInfo->${respMembers[i].name});\n`;
                        break;
                    case 'uint64':
                    case 'unsigned long long':
                    case 'int64':
                    case 'long long':
                        newDefine += `    sdk_set_int64(dataJson, "${respMembers[i].name}", pInfo->${respMembers[i].name});\n`;
                        break;
                    default:
                        if (is_enum_type(respMembers[i].type)) {
                            newDefine += `    sdk_set_int(dataJson, "${respMembers[i].name}", (int)(pInfo->${respMembers[i].name}));\n`;
                        } else {
                            newDefine += `    // TODO: pInfo->${respMembers[i].name} need special deal\n`;
                        }
                }
            }
        }
        // newDefine += '\n';
        // newDefine += '    // TODO: add your code here\n';
        // newDefine += '\n';
        newDefine += '    sdk_response_end();\n';
        newDefine += '    return ret;\n';
        newDefine += '}';
    }

    return newDefine;
}

function init_sdk_file_func_declare()
{
    const mod = get_mod_from_req_name(g_arg.reqName);
    const sReqName = get_req_name_without_mod();
    const sReqType = get_param_struct_name(g_arg.reqParams);

    let newClare = '';
    if (g_arg.needSdkIn) {
        newClare += '\n';
        newClare += `int ware_${MODS[mod].toLowerCase()}_${sReqName.toLowerCase()}_in(char *reqUrl, const char *jsonBuff, char *resp, int respLen, ${sReqType} *param);`;
    }
    if (g_arg.needSdkOut) {
        newClare += '\n';
        newClare += `int ware_${MODS[mod].toLowerCase()}_${sReqName.toLowerCase()}_out(char *reqUrl, void *data, int cnt, char *resp, int respLen);`;
    }
    return newClare;
}

function update_sdk_file()
{
    const mod = get_mod_from_req_name(g_arg.reqName);
    const cFile = 'src/libs/sdkp2p/sdk_util.c';
    const hFile = 'src/public/include/sdkp2p/sdk_util.h';

    update_file(cFile, '// request function define end', init_sdk_file_func_define());
    update_file(hFile, `// ${MODS[mod].toLowerCase()} request function declare end`, init_sdk_file_func_declare());

    return;
}

function init_ovf_file_func_define()
{
    const sReqName = get_req_name_without_mod();
    const sReqType = get_param_struct_name(g_arg.reqParams);
    const sRespType = get_param_struct_name(g_arg.respParams);
    const reqMembers = get_param_struct_members(g_arg.reqParams);
    const respMembers = get_param_struct_members(g_arg.respParams);

    let newDefine = '\n';
    if (is_get_req(sReqName)) {
        if (g_arg.respParams.length === 1) {
            newDefine += `int curl_${sReqName.toLowerCase()}(MS_CURL_INFO *info, ${respMembers[0].type} *resp)\n`;
        } else {
            newDefine += `int curl_${sReqName.toLowerCase()}(MS_CURL_INFO *info, ${sRespType} *resp)\n`;
        }
        newDefine += '{\n';
        newDefine += '    // TODO: Check if the params are correct\n';
        newDefine += '    snprintf(info->reqs, sizeof(info->reqs), "%s%s%s", /*IPC_OPERATPR_CGI*/, IPC_' + sReqName.toUpperCase().replace(/_IPC/g, '') + ', IPC_FORMAT_JSON);\n';
        newDefine += '\n';
        // newDefine += '    // TODO: add your code here\n';
        // newDefine += '\n';
        newDefine += '    return ms_curl_' + sReqName.toLowerCase() + '(info, resp);\n';
        newDefine += '}';
    } else { // set
        if (g_arg.reqParams.length === 1) {
            newDefine += `int curl_${sReqName.toLowerCase()}(MS_CURL_INFO *info, ${reqMembers[0].type} *param)\n`;
        } else {
            newDefine += `int curl_${sReqName.toLowerCase()}(MS_CURL_INFO *info, ${sReqType} *param)\n`;
        }  
        newDefine += '{\n';
        newDefine += '    // TODO: Check if the params are correct\n';
        newDefine += '    snprintf(info->reqs, sizeof(info->reqs), "%s%s%s", IPC_OPERATPR_CGI, IPC_' + sReqName.toUpperCase().replace(/_IPC/g, '') + ', IPC_FORMAT_JSON);\n';
        newDefine += '    curl_request_start();\n';

        if (g_arg.reqParams.length === 1) {
            if (reqMembers[0].arrSize) {
                newDefine += `    // TODO: param->${reqMembers[0].name} need special deal\n`;
            } else {
                switch (reqMembers[0].type.toLowerCase()) {
                    case 'int':
                        newDefine += `    curl_request_set_int(root, "${reqMembers[0].name}", *param);\n`;
                        break;
                    case 'char':
                        newDefine += `    curl_request_set_string(root, "${reqMembers[0].name}", param);\n`;
                        break;
                    default:
                        if (is_enum_type(reqMembers[0].type)) {
                            newDefine += `    curl_request_set_int(root, "${reqMembers[0].name}", (int)*param);\n`;
                        } else {
                            newDefine += `    // TODO: param->${reqMembers[0].name} need special deal\n`;
                        }
                        break;
                }
            }
        } else {
            for (let i = 0; i < reqMembers.length; i++) {
                if (reqMembers[i].arrSize) {
                    newDefine += `    // TODO: param->${reqMembers[i].name} need special deal\n`;
                } else {
                    if (reqMembers[i].name.toLowerCase() == 'chnid') {
                        continue;
                    }
                    switch (reqMembers[i].type.toLowerCase()) {
                        case 'int':
                            newDefine += `    curl_request_set_int(root, "${reqMembers[i].name}", param->${reqMembers[i].name});\n`;
                            break;
                        case 'char':
                            newDefine += `    curl_request_set_string(root, "${reqMembers[i].name}", param->${reqMembers[i].name});\n`;
                            break;
                        case 'double':
                            newDefine += `    curl_request_set_double(root, "${reqMembers[i].name}", param->${reqMembers[i].name});\n`;
                            break;
                        default:
                            if (is_enum_type(reqMembers[i].type)) {
                                newDefine += `    curl_request_set_int(root, "${reqMembers[i].name}", (int)param->${reqMembers[i].name});\n`;
                            } else {
                                newDefine += `    // TODO: param->${reqMembers[i].name} need special deal\n`;
                            }
                            break;
                    }
                }
            }
        }

        // newDefine += '\n';
        // newDefine += '    // TODO: add your code here\n';
        // newDefine += '\n';
        newDefine += '    curl_request_end();\n';
        newDefine += '    return 0;\n';
        newDefine += '}';
    }

    return newDefine;
}

function init_ovf_file_func_declare()
{
    const sReqName = get_req_name_without_mod();
    const sReqType = get_param_struct_name(g_arg.reqParams);
    const sRespType = get_param_struct_name(g_arg.respParams);
    const reqMembers = get_param_struct_members(g_arg.reqParams);
    const respMembers = get_param_struct_members(g_arg.respParams);

    let newDeclare = '';
    if (is_get_req(sReqName)) {
        if (g_arg.respParams.length === 1) {
            newDeclare += `int curl_${sReqName.toLowerCase()}(MS_CURL_INFO *info, ${respMembers[0].type} *resp);`;
        } else {
            newDeclare += `int curl_${sReqName.toLowerCase()}(MS_CURL_INFO *info, ${sRespType} *resp);`;
        }
    } else { // set
        if (g_arg.reqParams.length === 1) {
            newDeclare += `int curl_${sReqName.toLowerCase()}(MS_CURL_INFO *info, ${reqMembers[0].type} *param);`;
        } else {
            newDeclare += `int curl_${sReqName.toLowerCase()}(MS_CURL_INFO *info, ${sReqType} *param);`;
        }   
    }

    return newDeclare;
}

function init_ovf_file_msmap_func_define()
{
    const sReqName = get_req_name_without_mod();
    const sRespType = get_param_struct_name(g_arg.respParams);
    const reqMembers = get_param_struct_members(g_arg.respParams);
    const respMembers = get_param_struct_members(g_arg.respParams);

    let newDefine = '';
    if (is_get_req(sReqName)) {
        newDefine += `\n`;
        newDefine += `static int curl_${sReqName.toLowerCase().replace('get', 'recv')}_cb(void *req, void *resp)\n`;
        newDefine += '{\n';
        newDefine += '    curl_json_start();\n';
        if (g_arg.respParams.length === 1) {
            newDefine += `    ${respMembers[0].type} *param = (${respMembers[0].type} *)resp;\n`;
            if (respMembers[0].arrSize) {
                newDefine += `    // TODO: param->${respMembers[0].name} need special deal\n`;
            } else {
                switch (respMembers[0].type.toLowerCase()) {
                    case 'int':
                        newDefine += `    curl_json_get_int("${respMembers[0].name}", *param, MS_INVALID_VALUE);\n`;
                        break;
                    case 'char':
                        newDefine += `    curl_json_get_string("${respMembers[0].name}", param, "");\n`;
                        break;
                    default:
                        if (is_enum_type(respMembers[0].type)) {
                            newDefine += `    curl_json_get_int("${respMembers[0].name}", (int)*param, MS_INVALID_VALUE);\n`;
                        } else {
                            newDefine += `    // TODO: param->${respMembers[0].name} need special deal\n`;
                        }
                }
            }
        } else {
            newDefine += '    ' + sRespType + ' *param = (' + sRespType + ' *)resp;\n';
            for (let i = 0; i < reqMembers.length; i++) {
                if (reqMembers[i].arrSize) {
                    newDefine += `    // TODO: param->${reqMembers[i].name} need special deal\n`;
                } else {
                    if (reqMembers[i].name.toLowerCase() == 'chnid') {
                        continue;
                    }
                    switch (reqMembers[i].type.toLowerCase()) {
                        case 'int':
                            newDefine += `    curl_json_get_int("${reqMembers[i].name}", param->${reqMembers[i].name}, MS_INVALID_VALUE);\n`;
                            break;
                        case 'char':
                            newDefine += `    curl_json_get_string("${reqMembers[i].name}", param->${reqMembers[i].name}, "");\n`;
                            break;
                        default:
                            if (is_enum_type(respMembers[i].type)) {
                                newDefine += `    curl_json_get_int("${reqMembers[i].name}", (int)param->${reqMembers[i].name}, MS_INVALID_VALUE);\n`;
                            } else {
                                newDefine += `    // TODO: param->${reqMembers[i].name} need special deal\n`;
                            }
                            break;
                    }
                }
            }
        }

        // newDefine += '\n';
        // newDefine += '    // TODO: add your code here\n';
        // newDefine += '\n';
        newDefine += '    curl_json_end();\n';
        newDefine += '    return 0;\n';
        newDefine += '}\n';
        newDefine += '\n';
        if (g_arg.respParams.length === 1) {
            newDefine += `int ms_curl_${sReqName.toLowerCase()}(MS_CURL_INFO *info, ${respMembers[0].type} *resp)\n`;
        } else {
            newDefine += `int ms_curl_${sReqName.toLowerCase()}(MS_CURL_INFO *info, ${sRespType} *resp)\n`;
        }
        newDefine += '{\n';
        newDefine += '    return ms_curl_socket_to_ipc_safe2(info, resp, curl_' + sReqName.toLowerCase().replace('get', 'recv') + '_cb);\n';
        newDefine += '}';
    }

    return newDefine;
}

function init_ovf_file_macro(sFile, sTargetLine)
{

    const text = fs.readFileSync(sFile, 'utf8');
    const lines = text.split('\n');
    let targetLine = -1;

    for (let i = 0; i < lines.length; i++) {
        if (lines[i].includes(sTargetLine)) {
            targetLine = i;
            break;
        }
    }

    if (targetLine < 0) {
        console.error(`Can't find target line: ${sTargetLine}`);
        return -1;
    }


    const sReqName = get_req_name_without_mod();
    const macroName = `#define IPC_${sReqName.toUpperCase().replace(/_IPC/g, '')}`;
    const blankCnt = lines[targetLine - 1].indexOf('"') - macroName.length;
    const sBlank = ' '.repeat(blankCnt); // for format alignment

    const newMacro = `${macroName}${sBlank}"${sReqName.toLowerCase().replace(/_ipc/g, '').replace(/_/g, '.')}"`;
    return newMacro;
}

function init_ovf_file_msmap_func_declare()
{
    const sReqName = get_req_name_without_mod();
    const sRespType = get_param_struct_name(g_arg.respParams);
    const respMembers = get_param_struct_members(g_arg.respParams);

    let newDeclare = '';
    if (is_get_req(sReqName)) {
        if (g_arg.respParams.length === 1) {
            newDeclare += `int ms_curl_${sReqName.toLowerCase()}(MS_CURL_INFO *info, ${respMembers[0].type} *resp);`;
        } else {
            newDeclare += `int ms_curl_${sReqName.toLowerCase()}(MS_CURL_INFO *info, ${sRespType} *resp);`;
        }
    }

    return newDeclare;
}

function update_ovf_file()
{
    const cFile = 'src/libs/ovfcli/ovfcli.c';
    const hFile = 'src/public/include/ovfcli/ovfcli.h';
    const msmapCFile = 'src/libs/ovfcli/ovfcli_msmap.c';
    const msmapHFile = 'src/libs/ovfcli/ovfcli_msmap.h';

    update_file(cFile, '// request function define end', init_ovf_file_func_define());
    update_file(hFile, `// request function declare end`, init_ovf_file_func_declare());
    update_file(msmapCFile, `// request function define end`, init_ovf_file_msmap_func_define());
    update_file(msmapHFile, `// request macro end`, init_ovf_file_macro(msmapHFile, `// request macro end`));
    update_file(msmapHFile, `// request function declare end`, init_ovf_file_msmap_func_declare());

    return;
}

async function main() {
    console.log('[wcm] says hello to you!\n');
    let input = '';

    g_arg.reqName = await quest_singleline_input('Please enter the request name(such as get.camera.ipc.cap.image): ');
    console.log(`reqName = ${g_arg.reqName}`);
    // g_arg.reqName = 'set.camera.wcm';

    input = await quest_singleline_input('Please enter the request Id(see msg.h enum RequestFlag, such as 4028): ');
    g_arg.reqId = +input;
    if (isNaN(g_arg.reqId)) {
        console.log(`input = ${input} is not a number`);
        return;
    }
    console.log(`input = ${input}, reqId = ${g_arg.reqId}`);
    // g_arg.reqId = 4029;

    input = await quest_singleline_input('Need to generate sdk in function?(y/n): ');
    g_arg.needSdkIn = (input.toLowerCase() === 'y');
    console.log(`input = ${input}, needSdkIn = ${g_arg.needSdkIn}`);
    // g_arg.needSdkIn = true;

    input = await quest_singleline_input('Need to generate sdk out function?(y/n): ');
    g_arg.needSdkOut = (input.toLowerCase() === 'y');
    console.log(`input = ${input}, needSdkOut = ${g_arg.needSdkOut}`);
    // g_arg.needSdkOut = true;

    input = await quest_singleline_input('Need to generate core function?(y/n): ');
    g_arg.needCore = (input.toLowerCase() === 'y');
    console.log(`input = ${input}, needCore = ${g_arg.needCore}`);
    // g_arg.needCore = false;

    input = await quest_singleline_input('Need to generate onvif function?(y/n): ');
    g_arg.needOvf = (input.toLowerCase() === 'y');
    console.log(`input = ${input}, needOvf = ${g_arg.needOvf}`);
    // g_arg.needOvf = true;

    g_arg.reqParams = await quest_multiline_input('Please enter the request type(basic or struct) declaration, such as:\n' 
        + '1. int chnId;\n' 
        + '2. char username[32];\n' 
        + '3. typedef struct ReqPushmsgPictures {\n' 
        + '    Uint64 chnMask;\n' 
        + '    time_t time;\n' 
        + '    int width;\n' 
        + '    int height;\n' 
        + '} REQ_PUSHMSG_PICTURES_S;\n'
        + 'Enter blank line to end:\n'
    );
    console.log(`reqParams = ${g_arg.reqParams}`);

    g_arg.respParams = await quest_multiline_input('Please enter the response type(basic or struct) declaration, such as:\n' 
        + '1. int res;\n' 
        + '2. char username[32];\n' 
        + '3. typedef struct RespIpcAlarmstatus {\n' 
        + '    Uint64 respStatus;\n' 
        + '    Uint64 alarmstatuses[MAX_REAL_CAMERA];\n' 
        + '} RESP_IPC_ALARMSTATUS_S;\n'
        + 'Enter blank line to end:\n'
    );
    console.log(`respParams = ${g_arg.respParams}`);

    update_url_file();
    update_p2p_file();
    update_core_file();
    update_sdk_file();
    update_ovf_file();
}

main().catch((error) => {
    console.error(error);
});
