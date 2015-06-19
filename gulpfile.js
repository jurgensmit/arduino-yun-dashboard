/*
 * Gulp file for the Arduino Yún Dashboard application
 */
var gulp = require('gulp');
var args = require('yargs').argv;
var config = require('./gulp.config')();

var $ = require('gulp-load-plugins')({lazy: true});

/*
 * Analyze the JavaScript files with JSSC and JSHint
 */
gulp.task('analyze-code', function() {
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

/*
 * Deploy the web to the Atheros AR9331 Linux microcomputer on the Arduino Yún board
 */
gulp.task('deploy-web', ['analyze-code'], function() {
    log('Deploying web to Atheros AR9331');
    return gulp
        .src('./AtherosAR9331/**/*.*')
        .pipe($.plumber())
        .pipe($.if(args.verbose, $.print()))
        .pipe($.sftp(config.sftpOptions));
});

/*
 * The task to run when starting gulp without any specific task to run
 */
gulp.task('default', ['analyze-code']);

/*
 * Function to log a message to the console
 */
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
