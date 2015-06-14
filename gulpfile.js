var gulp = require('gulp');
var args = require('yargs').argv;
var config = require('./gulp.config')();

var $ = require('gulp-load-plugins')({lazy: true});

gulp.task('vet', function() {
    log('Analyzing source code with JSHint and JSCS');
    return gulp
        .src(config.alljs)
        .pipe($.plumber())
        .pipe($.if(args.verbose, $.print()))
        .pipe($.jscs())
        .pipe($.jshint())
        .pipe($.jshint.reporter('jshint-stylish', {verbose: true}))
        .pipe($.jshint.reporter('fail'));
});

// Default Task
gulp.task('default', ['vet']);

gulp.task('deploy-web', ['vet'], function() {
    return gulp
        .src('./AtherosAR9331/**/*.*')
        .pipe($.plumber())
        .pipe($.if(args.verbose, $.print()))
        .pipe($.sftp(config.sftpOptions));
});

function log(message) {
    if (typeof message === 'object') {
        for (var item in message) {
            if (message.hasOwnProperty(item)) {
                $.util.log($.util.colors.blue(message[item]));
            }
        }
    } else {
        $.util.log($.util.colors.blue(message));
    }
}
