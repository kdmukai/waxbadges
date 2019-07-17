const path = require('path');
const webpack = require('webpack');
const CopyWebpackPlugin = require('copy-webpack-plugin');


module.exports = {
    entry: './src/index.js',
    output: {
        filename: 'app.js',
        path: path.resolve(__dirname, 'build'),
        library: 'waxbadges_explorer'
    },
    module: {
        rules: [
            {
                test: /\.(js|jsx)$/,
                exclude: /node_modules/,
                use: {
                    loader: "babel-loader"
                }
            }
        ]
    },
    plugins: [
        new webpack.HotModuleReplacementPlugin(),
        new CopyWebpackPlugin([
            {
                from: './src/static',
                ignore: ['.DS_Store']
            }
        ])
    ],
    devServer: {
        contentBase: './build',
        hot: true,
        headers: {
            'Access-Control-Allow-Origin': '*'
        }
    }
};
