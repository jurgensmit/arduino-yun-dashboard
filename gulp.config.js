module.exports = function () {
    var config = {

        // all js to vet
        alljs: [
            './AtherosAR9331/public/**/*.js',
            './*.js'
        ],

        // sftp options
        sftpOptions: {
            host: '192.168.178.35',
            remotePath: '/root/',
            auth: 'keyMain'
        }
    };

    return config;
};
