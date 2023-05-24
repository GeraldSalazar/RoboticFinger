import 'package:flutter/material.dart';

class TitleWidget extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return Center(
      child: Text(
        'Number\nPad',
        style: TextStyle(
          fontSize: 42,
          fontWeight: FontWeight.bold,
          fontFamily: 'Roboto Mono',
          fontStyle: FontStyle.italic
        ),
        textAlign: TextAlign.center,
      ),
    );
  }
}
